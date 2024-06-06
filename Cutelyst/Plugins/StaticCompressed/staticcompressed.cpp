/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "staticcompressed_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Engine>
#include <Cutelyst/Request>
#include <Cutelyst/Response>
#include <array>
#include <chrono>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDataStream>
#include <QDateTime>
#include <QFile>
#include <QLockFile>
#include <QLoggingCategory>
#include <QMimeDatabase>
#include <QStandardPaths>

#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
#    include <brotli/encode.h>
#endif

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_STATICCOMPRESSED, "cutelyst.plugin.staticcompressed", QtWarningMsg)

StaticCompressed::StaticCompressed(Application *parent)
    : Plugin(parent)
    , d_ptr(new StaticCompressedPrivate)
{
    Q_D(StaticCompressed);
    d->includePaths.append(parent->config(u"root"_qs).toString());
}

StaticCompressed::StaticCompressed(Application *parent, const QVariantMap &defaultConfig)
    : Plugin(parent)
    , d_ptr(new StaticCompressedPrivate)
{
    Q_D(StaticCompressed);
    d->includePaths.append(parent->config(u"root"_qs).toString());
    d->defaultConfig = defaultConfig;
}

StaticCompressed::~StaticCompressed() = default;

void StaticCompressed::setIncludePaths(const QStringList &paths)
{
    Q_D(StaticCompressed);
    d->includePaths.clear();
    for (const QString &path : paths) {
        d->includePaths.append(QDir(path));
    }
}

void StaticCompressed::setDirs(const QStringList &dirs)
{
    Q_D(StaticCompressed);
    d->dirs = dirs;
}

void StaticCompressed::setServeDirsOnly(bool dirsOnly)
{
    Q_D(StaticCompressed);
    d->serveDirsOnly = dirsOnly;
}

bool StaticCompressed::setup(Application *app)
{
    Q_D(StaticCompressed);

    const QVariantMap config = app->engine()->config(u"Cutelyst_StaticCompressed_Plugin"_qs);
    const QString _defaultCacheDir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
        QLatin1String("/compressed-static");
    d->cacheDir.setPath(config
                            .value(u"cache_directory"_qs,
                                   d->defaultConfig.value(u"cache_directory"_qs, _defaultCacheDir))
                            .toString());

    if (Q_UNLIKELY(!d->cacheDir.exists())) {
        if (!d->cacheDir.mkpath(d->cacheDir.absolutePath())) {
            qCCritical(C_STATICCOMPRESSED)
                << "Failed to create cache directory for compressed static files at"
                << d->cacheDir.absolutePath();
            return false;
        }
    }

    qCInfo(C_STATICCOMPRESSED) << "Compressed cache directory:" << d->cacheDir.absolutePath();

    const QString _mimeTypes =
        config
            .value(u"mime_types"_qs,
                   d->defaultConfig.value(u"mime_types"_qs,
                                          u"text/css,application/javascript,text/javascript"_qs))
            .toString();
    qCInfo(C_STATICCOMPRESSED) << "MIME Types:" << _mimeTypes;
    d->mimeTypes = _mimeTypes.split(u',', Qt::SkipEmptyParts);

    const QString _suffixes =
        config
            .value(
                u"suffixes"_qs,
                d->defaultConfig.value(u"suffixes"_qs, u"js.map,css.map,min.js.map,min.css.map"_qs))
            .toString();
    qCInfo(C_STATICCOMPRESSED) << "Suffixes:" << _suffixes;
    d->suffixes = _suffixes.split(u',', Qt::SkipEmptyParts);

    d->checkPreCompressed = config
                                .value(u"check_pre_compressed"_qs,
                                       d->defaultConfig.value(u"check_pre_compressed"_qs, true))
                                .toBool();
    qCInfo(C_STATICCOMPRESSED) << "Check for pre-compressed files:" << d->checkPreCompressed;

    d->onTheFlyCompression = config
                                 .value(u"on_the_fly_compression"_qs,
                                        d->defaultConfig.value(u"on_the_fly_compression"_qs, true))
                                 .toBool();
    qCInfo(C_STATICCOMPRESSED) << "Compress static files on the fly:" << d->onTheFlyCompression;

    QStringList supportedCompressions{u"deflate"_qs, u"gzip"_qs};
    d->loadZlibConfig(config);

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
    d->loadZopfliConfig(config);
    supportedCompressions << u"zopfli"_qs;
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    d->loadBrotliConfig(config);
    supportedCompressions << u"brotli"_qs;
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
    if (Q_UNLIKELY(!d->loadZstdConfig(config))) {
        return false;
    }
    supportedCompressions << u"zstd"_qs;
#endif

    qCInfo(C_STATICCOMPRESSED) << "Supported compressions:" << supportedCompressions.join(u',');
    qCInfo(C_STATICCOMPRESSED) << "Include paths:" << d->includePaths;

    connect(app, &Application::beforePrepareAction, this, [d](Context *c, bool *skipMethod) {
        d->beforePrepareAction(c, skipMethod);
    });

    return true;
}

void StaticCompressedPrivate::beforePrepareAction(Context *c, bool *skipMethod)
{
    if (*skipMethod) {
        return;
    }

    // TODO mid(1) quick fix for path now having leading slash
    const QString path = c->req()->path().mid(1);

    for (const QString &dir : std::as_const(dirs)) {
        if (path.startsWith(dir)) {
            if (!locateCompressedFile(c, path)) {
                Response *res = c->response();
                res->setStatus(Response::NotFound);
                res->setContentType("text/html"_qba);
                res->setBody(u"File not found: "_qs + path);
            }

            *skipMethod = true;
            return;
        }
    }

    if (serveDirsOnly) {
        return;
    }

    const QRegularExpression _re        = re; // Thread-safe
    const QRegularExpressionMatch match = _re.match(path);
    if (match.hasMatch() && locateCompressedFile(c, path)) {
        *skipMethod = true;
    }
}

bool StaticCompressedPrivate::locateCompressedFile(Context *c, const QString &relPath) const
{
    for (const QDir &includePath : includePaths) {
        qCDebug(C_STATICCOMPRESSED)
            << "Trying to find" << relPath << "in" << includePath.absolutePath();
        const QString path = includePath.absoluteFilePath(relPath);
        const QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            Response *res                   = c->res();
            const QDateTime currentDateTime = fileInfo.lastModified();
            if (!c->req()->headers().ifModifiedSince(currentDateTime)) {
                res->setStatus(Response::NotModified);
                return true;
            }

            static QMimeDatabase db;
            // use the extension to match to be faster
            const QMimeType mimeType = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
            QByteArray contentEncoding;
            QString compressedPath;
            QByteArray _mimeTypeName;

            if (mimeType.isValid()) {

                // QMimeDatabase might not find the correct mime type for some specific types
                // especially for map files for CSS and JS
                if (mimeType.isDefault()) {
                    if (path.endsWith(u"css.map", Qt::CaseInsensitive) ||
                        path.endsWith(u"js.map", Qt::CaseInsensitive)) {
                        _mimeTypeName = "application/json"_qba;
                    }
                }

                if (mimeTypes.contains(mimeType.name(), Qt::CaseInsensitive) ||
                    suffixes.contains(fileInfo.completeSuffix(), Qt::CaseInsensitive)) {

                    const auto acceptEncoding = c->req()->header("Accept-Encoding");

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
                    if (acceptEncoding.contains("zstd")) {
                        compressedPath = locateCacheFile(path, currentDateTime, Zstd);
                        if (!compressedPath.isEmpty()) {
                            qCDebug(C_STATICCOMPRESSED)
                                << "Serving zstd compressed data from" << compressedPath;
                            contentEncoding = "zstd"_qba;
                        }
                    } else
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
                        if (acceptEncoding.contains("br")) {
                        compressedPath = locateCacheFile(path, currentDateTime, Brotli);
                        if (!compressedPath.isEmpty()) {
                            qCDebug(C_STATICCOMPRESSED)
                                << "Serving brotli compressed data from" << compressedPath;
                            contentEncoding = "br"_qba;
                        }
                    } else
#endif
                        if (acceptEncoding.contains("gzip")) {
                        compressedPath =
                            locateCacheFile(path, currentDateTime, zopfli.use ? ZopfliGzip : Gzip);
                        if (!compressedPath.isEmpty()) {
                            qCDebug(C_STATICCOMPRESSED)
                                << "Serving" << (zopfli.use ? "zopfli" : "gzip")
                                << "compressed data from" << compressedPath;
                            contentEncoding = "gzip"_qba;
                        }
                    } else if (acceptEncoding.contains("deflate")) {
                        compressedPath = locateCacheFile(
                            path, currentDateTime, zopfli.use ? ZopfliDeflate : Deflate);
                        if (!compressedPath.isEmpty()) {
                            qCDebug(C_STATICCOMPRESSED)
                                << "Serving deflate compressed data from" << compressedPath;
                            contentEncoding = "deflate"_qba;
                        }
                    }
                }
            }

            // Response::setBody() will take the ownership
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            QFile *file = !compressedPath.isEmpty() ? new QFile(compressedPath) : new QFile(path);
            if (file->open(QFile::ReadOnly)) {
                qCDebug(C_STATICCOMPRESSED) << "Serving" << path;
                Headers &headers = res->headers();

                // set our open file
                res->setBody(file);

                // if we have a mime type determine from the extension,
                // do not use the name from the mime database
                if (!_mimeTypeName.isEmpty()) {
                    headers.setContentType(_mimeTypeName);
                } else if (mimeType.isValid()) {
                    headers.setContentType(mimeType.name().toLatin1());
                }
                headers.setContentLength(file->size());

                headers.setLastModified(currentDateTime);
                // Tell Firefox & friends its OK to cache, even over SSL
                headers.setCacheControl("public"_qba);

                if (!contentEncoding.isEmpty()) {
                    // serve correct encoding type
                    headers.setContentEncoding(contentEncoding);

                    qCDebug(C_STATICCOMPRESSED)
                        << "Encoding:" << headers.contentEncoding() << "Size:" << file->size()
                        << "Original Size:" << fileInfo.size();

                    // force proxies to cache compressed and non-compressed files separately
                    headers.pushHeader("Vary"_qba, "Accept-Encoding"_qba);
                }

                return true;
            }

            qCWarning(C_STATICCOMPRESSED) << "Could not serve" << path << file->errorString();
            delete file;
            return false;
        }
    }

    qCWarning(C_STATICCOMPRESSED) << "File not found" << relPath;
    return false;
}

QString StaticCompressedPrivate::locateCacheFile(const QString &origPath,
                                                 const QDateTime &origLastModified,
                                                 Compression compression) const
{
    QString compressedPath;

    QString suffix;

    switch (compression) {
    case ZopfliGzip:
    case Gzip:
        suffix = u".gz"_qs;
        break;
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
    case Zstd:
        suffix = u".zst"_qs;
        break;
#endif
#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    case Brotli:
        suffix = u".br"_qs;
        break;
#endif
    case ZopfliDeflate:
    case Deflate:
        suffix = u".deflate"_qs;
        break;
    default:
        Q_ASSERT_X(false, "locate cache file", "invalid compression type");
        break;
    }

    if (checkPreCompressed) {
        const QFileInfo origCompressed(origPath + suffix);
        if (origCompressed.exists()) {
            compressedPath = origCompressed.absoluteFilePath();
            return compressedPath;
        }
    }

    if (onTheFlyCompression) {

        const QString path = cacheDir.absoluteFilePath(
            QString::fromLatin1(
                QCryptographicHash::hash(origPath.toUtf8(), QCryptographicHash::Md5).toHex()) +
            suffix);
        const QFileInfo info(path);

        if (info.exists() && (info.lastModified() > origLastModified)) {
            compressedPath = path;
        } else {
            QLockFile lock(path + QLatin1String(".lock"));
            if (lock.tryLock(std::chrono::milliseconds{10})) {
                switch (compression) {
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
                case Zstd:
                    if (compressZstd(origPath, path)) {
                        compressedPath = path;
                    }
                    break;
#endif
#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
                case Brotli:
                    if (compressBrotli(origPath, path)) {
                        compressedPath = path;
                    }
                    break;
#endif
                case ZopfliGzip:
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
                    if (compressZopfli(origPath, path, ZopfliFormat::ZOPFLI_FORMAT_GZIP)) {
                        compressedPath = path;
                    }
                    break;
#endif
                case Gzip:
                    if (compressGzip(origPath, path, origLastModified)) {
                        compressedPath = path;
                    }
                    break;
                case ZopfliDeflate:
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
                    if (compressZopfli(origPath, path, ZopfliFormat::ZOPFLI_FORMAT_ZLIB)) {
                        compressedPath = path;
                    }
                    break;
#endif
                case Deflate:
                    if (compressDeflate(origPath, path)) {
                        compressedPath = path;
                    }
                    break;
                default:
                    break;
                }
                lock.unlock();
            }
        }
    }

    return compressedPath;
}

void StaticCompressedPrivate::loadZlibConfig(const QVariantMap &conf)
{
    bool ok = false;
    zlib.compressionLevel =
        conf.value(u"zlib_compression_level"_qs,
                   defaultConfig.value(u"zlib_compression_level"_qs, zlib.compressionLevelDefault))
            .toInt(&ok);

    if (!ok || zlib.compressionLevel < zlib.compressionLevelMin ||
        zlib.compressionLevel > zlib.compressionLevelMax) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value set for zlib_compression_level. Value hat to be between "
            << zlib.compressionLevelMin << " and " << zlib.compressionLevelMax
            << " inclusive. Using default value " << zlib.compressionLevelDefault;
        zlib.compressionLevel = zlib.compressionLevelDefault;
    }
}

static constexpr std::array<quint32, 256> crc32Tab = []() {
    std::array<quint32, 256> tab{0};
    for (std::size_t n = 0; n < 256; n++) {
        auto c = static_cast<quint32>(n);
        for (int k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xedb88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        tab[n] = c;
    }
    return tab;
}();

quint32 updateCRC32(unsigned char ch, quint32 crc)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    return crc32Tab[(crc ^ ch) & 0xff] ^ (crc >> 8);
}

quint32 crc32buf(const QByteArray &data)
{
    return ~std::accumulate(data.begin(),
                            data.end(),
                            quint32(0xFFFFFFFF), // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                            [](quint32 oldcrc32, char buf) {
        return updateCRC32(static_cast<unsigned char>(buf), oldcrc32);
    });
}

bool StaticCompressedPrivate::compressGzip(const QString &inputPath,
                                           const QString &outputPath,
                                           const QDateTime &origLastModified) const
{
    qCDebug(C_STATICCOMPRESSED) << "Compressing" << inputPath << "with gzip to" << outputPath;

    QFile input(inputPath);
    if (Q_UNLIKELY(!input.open(QIODevice::ReadOnly))) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not open input file to compress with gzip:" << inputPath;
        return false;
    }

    const QByteArray data = input.readAll();
    if (Q_UNLIKELY(data.isEmpty())) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not read input file or input file is empty:" << inputPath;
        input.close();
        return false;
    }

    QByteArray compressedData = qCompress(data, zlib.compressionLevel);
    input.close();

    QFile output(outputPath);
    if (Q_UNLIKELY(!output.open(QIODevice::WriteOnly))) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not open output file to compress with gzip:" << outputPath;
        return false;
    }

    if (Q_UNLIKELY(compressedData.isEmpty())) {
        qCWarning(C_STATICCOMPRESSED)
            << "Failed to compress file with gzip, compressed data is empty:" << inputPath;
        if (output.exists()) {
            if (Q_UNLIKELY(!output.remove())) {
                qCWarning(C_STATICCOMPRESSED)
                    << "Can not remove invalid compressed gzip file:" << outputPath;
            }
        }
        return false;
    }

    // Strip the first six bytes (a 4-byte length put on by qCompress and a 2-byte zlib header)
    // and the last four bytes (a zlib integrity check).
    compressedData.remove(0, 6);
    compressedData.chop(4);

    QByteArray header;
    QDataStream headerStream(&header, QIODevice::WriteOnly);
    // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
    // prepend a generic 10-byte gzip header (see RFC 1952)
    headerStream << quint8(0x1f) << quint8(0x8b) // ID1 and ID2
                 << quint8(8)                    // CM / Compression Mode (8 = deflate)
                 << quint8(0)                    // FLG / flags
                 << static_cast<quint32>(origLastModified.toSecsSinceEpoch())
                 << quint8(0) // XFL / extra flags
#if defined Q_OS_UNIX
                 << quint8(3);
#elif defined Q_OS_MACOS
                 << quint8(7);
#elif defined Q_OS_WIN
                 << quint8(11);
#else
                 << quint8(255);
#endif
    // NOLINTEND(cppcoreguidelines-avoid-magic-numbers)

    // append a four-byte CRC-32 of the uncompressed data
    // append 4 bytes uncompressed input size modulo 2^32
    auto crc    = crc32buf(data);
    auto inSize = data.size();
    QByteArray footer;
    QDataStream footerStream(&footer, QIODevice::WriteOnly);
    footerStream << static_cast<quint8>(crc % 256) << static_cast<quint8>((crc >> 8) % 256)
                 << static_cast<quint8>((crc >> 16) % 256) << static_cast<quint8>((crc >> 24) % 256)
                 << static_cast<quint8>(inSize % 256) << static_cast<quint8>((inSize >> 8) % 256)
                 << static_cast<quint8>((inSize >> 16) % 256)
                 << static_cast<quint8>((inSize >> 24) % 256);

    if (Q_UNLIKELY(output.write(header + compressedData + footer) < 0)) {
        qCCritical(C_STATICCOMPRESSED).nospace()
            << "Failed to write compressed gzip file " << inputPath << ": " << output.errorString();
        return false;
    }

    return true;
}

bool StaticCompressedPrivate::compressDeflate(const QString &inputPath,
                                              const QString &outputPath) const
{
    qCDebug(C_STATICCOMPRESSED) << "Compressing" << inputPath << "with deflate to" << outputPath;

    QFile input(inputPath);
    if (Q_UNLIKELY(!input.open(QIODevice::ReadOnly))) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not open input file to compress with deflate:" << inputPath;
        return false;
    }

    const QByteArray data = input.readAll();
    if (Q_UNLIKELY(data.isEmpty())) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not read input file or input file is empty:" << inputPath;
        input.close();
        return false;
    }

    QByteArray compressedData = qCompress(data, zlib.compressionLevel);
    input.close();

    QFile output(outputPath);
    if (Q_UNLIKELY(!output.open(QIODevice::WriteOnly))) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not open output file to compress with deflate:" << outputPath;
        return false;
    }

    if (Q_UNLIKELY(compressedData.isEmpty())) {
        qCWarning(C_STATICCOMPRESSED)
            << "Failed to compress file with deflate, compressed data is empty:" << inputPath;
        if (output.exists()) {
            if (Q_UNLIKELY(!output.remove())) {
                qCWarning(C_STATICCOMPRESSED)
                    << "Can not remove invalid compressed deflate file:" << outputPath;
            }
        }
        return false;
    }

    // Strip the first four bytes (a 4-byte length header put on by qCompress)
    compressedData.remove(0, 4);

    if (Q_UNLIKELY(output.write(compressedData) < 0)) {
        qCCritical(C_STATICCOMPRESSED).nospace() << "Failed to write compressed deflate file "
                                                 << inputPath << ": " << output.errorString();
        return false;
    }

    return true;
}

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
void StaticCompressedPrivate::loadZopfliConfig(const QVariantMap &conf)
{
    zopfli.use =
        conf.value(u"use_zopfli"_qs, defaultConfig.value(u"use_zopfli"_qs, false)).toBool();
    if (zopfli.use) {
        ZopfliInitOptions(&zopfli.options);
        bool ok = false;
        zopfli.options.numiterations =
            conf.value(u"zopfli_iterations"_qs,
                       defaultConfig.value(u"zopfli_iterations"_qs, zopfli.iterationsDefault))
                .toInt(&ok);
        if (!ok || zopfli.options.numiterations < zopfli.iterationsMin) {
            qCWarning(C_STATICCOMPRESSED).nospace()
                << "Invalid value set for zopfli_iterations. Value has to to be an integer value "
                   "greater than or equal to "
                << zopfli.iterationsMin << ". Using default value " << zopfli.iterationsDefault;
            zopfli.options.numiterations = zopfli.iterationsDefault;
        }
    }
}

bool StaticCompressedPrivate::compressZopfli(const QString &inputPath,
                                             const QString &outputPath,
                                             ZopfliFormat format) const
{
    qCDebug(C_STATICCOMPRESSED) << "Compressing" << inputPath << "with zopfli to" << outputPath;

    QFile input(inputPath);
    if (Q_UNLIKELY(!input.open(QIODevice::ReadOnly))) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not open input file to compress with zopfli:" << inputPath;
        return false;
    }

    const QByteArray data = input.readAll();
    if (Q_UNLIKELY(data.isEmpty())) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not read input file or input file is empty:" << inputPath;
        return false;
    }

    input.close();

    unsigned char *out{nullptr};
    size_t outSize{0};

    ZopfliCompress(&zopfli.options,
                   format,
                   reinterpret_cast<const unsigned char *>(data.constData()),
                   data.size(),
                   &out,
                   &outSize);

    if (Q_UNLIKELY(outSize <= 0)) {
        qCWarning(C_STATICCOMPRESSED)
            << "Failed to compress file with zopfli, compressed data is empty:" << inputPath;
        free(out);
        return false;
    }

    QFile output{outputPath};
    if (Q_UNLIKELY(!output.open(QIODeviceBase::WriteOnly))) {
        qCWarning(C_STATICCOMPRESSED) << "Failed to open output file" << outputPath
                                      << "for zopfli compression:" << output.errorString();
        free(out);
        return false;
    }

    if (Q_UNLIKELY(output.write(reinterpret_cast<const char *>(out), outSize) < 0)) {
        if (output.exists()) {
            if (Q_UNLIKELY(!output.remove())) {
                qCWarning(C_STATICCOMPRESSED)
                    << "Can not remove invalid compressed zopfli file:" << outputPath;
            }
        }
        qCWarning(C_STATICCOMPRESSED) << "Failed to write zopfli compressed data to output file"
                                      << outputPath << ":" << output.errorString();
        free(out);
        return false;
    }

    free(out);

    return true;
}
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
void StaticCompressedPrivate::loadBrotliConfig(const QVariantMap &conf)
{
    bool ok = false;
    brotli.qualityLevel =
        conf.value(u"brotli_quality_level"_qs,
                   defaultConfig.value(u"brotli_quality_level"_qs, brotli.qualityLevelDefault))
            .toInt(&ok);

    if (!ok || brotli.qualityLevel < BROTLI_MIN_QUALITY ||
        brotli.qualityLevel > BROTLI_MAX_QUALITY) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value for brotli_quality_level. "
               "Has to be an integer value between "
            << BROTLI_MIN_QUALITY << " and " << BROTLI_MAX_QUALITY
            << " inclusive. Using default value " << brotli.qualityLevelDefault;
        brotli.qualityLevel = brotli.qualityLevelDefault;
    }
}

bool StaticCompressedPrivate::compressBrotli(const QString &inputPath,
                                             const QString &outputPath) const
{
    qCDebug(C_STATICCOMPRESSED) << "Compressing" << inputPath << "with brotli to" << outputPath;

    QFile input(inputPath);
    if (Q_UNLIKELY(!input.open(QIODevice::ReadOnly))) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not open input file to compress with brotli:" << inputPath;
        return false;
    }

    const QByteArray data = input.readAll();
    if (Q_UNLIKELY(data.isEmpty())) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not read input file or input file is empty:" << inputPath;
        return false;
    }

    input.close();

    size_t outSize = BrotliEncoderMaxCompressedSize(static_cast<size_t>(data.size()));
    if (Q_UNLIKELY(outSize == 0)) {
        qCWarning(C_STATICCOMPRESSED) << "Needed output buffer too large to compress input of size"
                                      << data.size() << "with brotli";
        return false;
    }
    QByteArray outData{static_cast<qsizetype>(outSize), Qt::Uninitialized};

    const auto in = reinterpret_cast<const uint8_t *>(data.constData());
    auto out      = reinterpret_cast<uint8_t *>(outData.data());

    const BROTLI_BOOL status = BrotliEncoderCompress(brotli.qualityLevel,
                                                     BROTLI_DEFAULT_WINDOW,
                                                     BROTLI_DEFAULT_MODE,
                                                     data.size(),
                                                     in,
                                                     &outSize,
                                                     out);
    if (Q_UNLIKELY(status != BROTLI_TRUE)) {
        qCWarning(C_STATICCOMPRESSED) << "Failed to compress" << inputPath << "with brotli";
        return false;
    }

    outData.resize(static_cast<qsizetype>(outSize));

    QFile output{outputPath};
    if (Q_UNLIKELY(!output.open(QIODeviceBase::WriteOnly))) {
        qCWarning(C_STATICCOMPRESSED) << "Failed to open output file" << outputPath
                                      << "for brotli compression:" << output.errorString();
        return false;
    }

    if (Q_UNLIKELY(output.write(outData) < 0)) {
        if (output.exists()) {
            if (Q_UNLIKELY(!output.remove())) {
                qCWarning(C_STATICCOMPRESSED)
                    << "Can not remove invalid compressed brotli file:" << outputPath;
            }
        }
        qCWarning(C_STATICCOMPRESSED) << "Failed to write brotli compressed data to output file"
                                      << outputPath << ":" << output.errorString();
        return false;
    }

    return true;
}
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
bool StaticCompressedPrivate::loadZstdConfig(const QVariantMap &conf)
{
    zstd.ctx = ZSTD_createCCtx();
    if (!zstd.ctx) {
        qCCritical(C_STATICCOMPRESSED) << "Failed to create Zstandard compression context";
        return false;
    }

    bool ok = false;

    zstd.compressionLevel =
        conf.value(u"zstd_compression_level"_qs,
                   defaultConfig.value(u"zstd_compression_level"_qs, zstd.compressionLevelDefault))
            .toInt(&ok);
    if (!ok || zstd.compressionLevel < ZSTD_minCLevel() ||
        zstd.compressionLevel > ZSTD_maxCLevel()) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value for zstd_compression_level. Has to be an integer value between "
            << ZSTD_minCLevel() << " and " << ZSTD_maxCLevel() << " inclusive. Using default value "
            << zstd.compressionLevelDefault;
        zstd.compressionLevel = zstd.compressionLevelDefault;
    }

    return true;
}

bool StaticCompressedPrivate::compressZstd(const QString &inputPath,
                                           const QString &outputPath) const
{
    qCDebug(C_STATICCOMPRESSED) << "Compressing" << inputPath << "with zstd to" << outputPath;

    QFile input{inputPath};
    if (Q_UNLIKELY(!input.open(QIODeviceBase::ReadOnly))) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not open input file to compress with zstd:" << inputPath;
        return false;
    }

    const QByteArray inData = input.readAll();
    if (Q_UNLIKELY(inData.isEmpty())) {
        qCWarning(C_STATICCOMPRESSED)
            << "Can not read input file or input file is empty:" << inputPath;
        return false;
    }

    input.close();

    const size_t outBufSize = ZSTD_compressBound(static_cast<size_t>(inData.size()));
    if (Q_UNLIKELY(ZSTD_isError(outBufSize) == 1)) {
        qCWarning(C_STATICCOMPRESSED)
            << "Failed to compress" << inputPath << "with zstd:" << ZSTD_getErrorName(outBufSize);
        return false;
    }
    QByteArray outData{static_cast<qsizetype>(outBufSize), Qt::Uninitialized};

    auto outDataP = static_cast<void *>(outData.data());
    auto inDataP  = static_cast<const void *>(inData.constData());

    const size_t outSize = ZSTD_compressCCtx(
        zstd.ctx, outDataP, outBufSize, inDataP, inData.size(), zstd.compressionLevel);
    if (Q_UNLIKELY(ZSTD_isError(outSize) == 1)) {
        qCWarning(C_STATICCOMPRESSED)
            << "Failed to compress" << inputPath << "with zstd:" << ZSTD_getErrorName(outSize);
        return false;
    }

    outData.resize(static_cast<qsizetype>(outSize));

    QFile output{outputPath};
    if (Q_UNLIKELY(!output.open(QIODeviceBase::WriteOnly))) {
        qCWarning(C_STATICCOMPRESSED) << "Failed to open output file" << outputPath
                                      << "for zstd compression:" << output.errorString();
        return false;
    }

    if (Q_UNLIKELY(output.write(outData) < 0)) {
        if (output.exists()) {
            if (Q_UNLIKELY(!output.remove())) {
                qCWarning(C_STATICCOMPRESSED)
                    << "Can not remove invalid compressed zstd file:" << outputPath;
            }
        }
        qCWarning(C_STATICCOMPRESSED) << "Failed to write zstd compressed data to output file"
                                      << outputPath << ":" << output.errorString();
        return false;
    }

    return true;
}
#endif

#include "moc_staticcompressed.cpp"
