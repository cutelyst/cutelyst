/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "staticcompressed_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Engine>
#include <Cutelyst/Request>
#include <Cutelyst/Response>
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

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
#    include <zopfli.h>
#endif

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

bool StaticCompressed::setup(Application *app)
{
    Q_D(StaticCompressed);

    const QVariantMap config = app->engine()->config(u"Cutelyst_StaticCompressed_Plugin"_qs);
    const QString _defaultCacheDir =
        QStandardPaths::writableLocation(QStandardPaths::CacheLocation) +
        QLatin1String("/compressed-static");
    d->cacheDir.setPath(config.value(u"cache_directory"_qs, _defaultCacheDir).toString());

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
        config.value(u"mime_types"_qs, u"text/css,application/javascript"_qs).toString();
    qCInfo(C_STATICCOMPRESSED) << "MIME Types:" << _mimeTypes;
    d->mimeTypes = _mimeTypes.split(u',', Qt::SkipEmptyParts);

    const QString _suffixes =
        config.value(u"suffixes"_qs, u"js.map,css.map,min.js.map,min.css.map"_qs).toString();
    qCInfo(C_STATICCOMPRESSED) << "Suffixes:" << _suffixes;
    d->suffixes = _suffixes.split(u',', Qt::SkipEmptyParts);

    d->checkPreCompressed = config.value(u"check_pre_compressed"_qs, true).toBool();
    qCInfo(C_STATICCOMPRESSED) << "Check for pre-compressed files:" << d->checkPreCompressed;

    d->onTheFlyCompression = config.value(u"on_the_fly_compression"_qs, true).toBool();
    qCInfo(C_STATICCOMPRESSED) << "Compress static files on the fly:" << d->onTheFlyCompression;

    QStringList supportedCompressions{u"deflate"_qs, u"gzip"_qs};

    bool ok                 = false;
    d->zlibCompressionLevel = config
                                  .value(u"zlib_compression_level"_qs,
                                         StaticCompressedPrivate::zlibCompressionLevelDefault)
                                  .toInt(&ok);
    if (!ok) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value set for zlib_compression_level. "
               "Has to to be an integer value between "
            << StaticCompressedPrivate::zlibCompressionLevelMin << " and "
            << StaticCompressedPrivate::zlibCompressionLevelMax
            << " inclusive. Using default value "
            << StaticCompressedPrivate::zlibCompressionLevelDefault;
    }

    if (d->zlibCompressionLevel < StaticCompressedPrivate::zlibCompressionLevelMin ||
        d->zlibCompressionLevel > StaticCompressedPrivate::zlibCompressionLevelMax) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value " << d->zlibCompressionLevel
            << " set for zlib_compression_level. Value hat to be between "
            << StaticCompressedPrivate::zlibCompressionLevelMin << " and "
            << StaticCompressedPrivate::zlibCompressionLevelMax
            << " inclusive. Using default value "
            << StaticCompressedPrivate::zlibCompressionLevelDefault;
        d->zlibCompressionLevel = StaticCompressedPrivate::zlibCompressionLevelDefault;
    }

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
    d->zopfliIterations =
        config.value(u"zopfli_iterations"_qs, StaticCompressedPrivate::zopfliIterationsDefault)
            .toInt(&ok);
    if (!ok) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value for zopfli_iterations. "
               "Has to be an integer value greater than or equal to "
            << StaticCompressedPrivate::zopfliIterationsMin << ". Using default value "
            << StaticCompressedPrivate::zopfliIterationsDefault;
        d->zopfliIterations = StaticCompressedPrivate::zopfliIterationsDefault;
    }

    if (d->zopfliIterations < StaticCompressedPrivate::zopfliIterationsMin) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value " << d->zopfliIterations
            << " set for zopfli_iterations. Value has to to be greater than or equal to "
            << StaticCompressedPrivate::zopfliIterationsMin << ". Using default value "
            << StaticCompressedPrivate::zopfliIterationsDefault;
        d->zopfliIterations = StaticCompressedPrivate::zopfliIterationsDefault;
    }
    d->useZopfli = config.value(u"use_zopfli"_qs, false).toBool();
    supportedCompressions << u"zopfli"_qs;
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    d->brotliQualityLevel =
        config.value(u"brotli_quality_level"_qs, StaticCompressedPrivate::brotliQualityLevelDefault)
            .toInt(&ok);
    if (!ok) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value for brotli_quality_level. "
               "Has to be an integer value between "
            << BROTLI_MIN_QUALITY << " and " << BROTLI_MAX_QUALITY
            << " inclusive. Using default value "
            << StaticCompressedPrivate::brotliQualityLevelDefault;
        d->brotliQualityLevel = StaticCompressedPrivate::brotliQualityLevelDefault;
    }

    if (d->brotliQualityLevel < BROTLI_MIN_QUALITY || d->brotliQualityLevel > BROTLI_MAX_QUALITY) {
        qCWarning(C_STATICCOMPRESSED).nospace()
            << "Invalid value " << d->brotliQualityLevel
            << " set for brotli_quality_level. Value has to be between " << BROTLI_MIN_QUALITY
            << " and " << BROTLI_MAX_QUALITY << " inclusive. Using default value "
            << StaticCompressedPrivate::brotliQualityLevelDefault;
        d->brotliQualityLevel = StaticCompressedPrivate::brotliQualityLevelDefault;
    }
    supportedCompressions << u"brotli"_qs;
#endif

    qCInfo(C_STATICCOMPRESSED) << "Supported compressions:" << supportedCompressions.join(u',');

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

    const QString path           = c->req()->path();
    const QRegularExpression _re = re; // Thread-safe

    for (const QString &dir : dirs) {
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

    const QRegularExpressionMatch match = _re.match(path);
    if (match.hasMatch() && locateCompressedFile(c, path)) {
        *skipMethod = true;
    }
}

bool StaticCompressedPrivate::locateCompressedFile(Context *c, const QString &relPath) const
{
    for (const QDir &includePath : includePaths) {
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
                    qCDebug(C_STATICCOMPRESSED) << "Accept-Encoding:" << acceptEncoding;

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
                            locateCacheFile(path, currentDateTime, useZopfli ? Zopfli : Gzip);
                        if (!compressedPath.isEmpty()) {
                            qCDebug(C_STATICCOMPRESSED)
                                << "Serving" << (useZopfli ? "zopfli" : "gzip")
                                << "compressed data from" << compressedPath;
                            contentEncoding = "gzip"_qba;
                        }
                    } else if (acceptEncoding.contains("deflate")) {
                        compressedPath = locateCacheFile(path, currentDateTime, Deflate);
                        if (!compressedPath.isEmpty()) {
                            qCDebug(C_STATICCOMPRESSED)
                                << "Serving deflate compressed data from" << compressedPath;
                            contentEncoding = "deflate"_qba;
                        }
                    }
                }
            }

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
    case Zopfli:
    case Gzip:
        suffix = u".gz"_qs;
        break;
#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    case Brotli:
        suffix = u".br"_qs;
        break;
#endif
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
#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
                case Brotli:
                    if (compressBrotli(origPath, path)) {
                        compressedPath = path;
                    }
                    break;
#endif
                case Zopfli:
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
                    if (compressZopfli(origPath, path)) {
                        compressedPath = path;
                    }
                    break;
#endif
                case Gzip:
                    if (compressGzip(origPath, path, origLastModified)) {
                        compressedPath = path;
                    }
                    break;
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

// clang-format off
static const quint32 crc_32_tab[] = { /* CRC polynomial 0xedb88320 */
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
    0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
    0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
    0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
    0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
    0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
    0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
    0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
    0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
    0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
    0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
    0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
    0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
    0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
    0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
    0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
    0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
    0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
    0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
    0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
    0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
    0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};
// clang-format on

quint32 updateCRC32(unsigned char ch, quint32 crc)
{
    return (crc_32_tab[((crc) ^ (quint8(ch))) & 0xff] ^ ((crc) >> 8));
}

quint32 crc32buf(const QByteArray &data)
{
    return ~std::accumulate(data.begin(),
                            data.end(),
                            quint32(0xFFFFFFFF),
                            [](quint32 oldcrc32, char buf) { return updateCRC32(buf, oldcrc32); });
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

    QByteArray compressedData = qCompress(data, zlibCompressionLevel);
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
    // prepend a generic 10-byte gzip header (see RFC 1952)
    headerStream << quint16(0x1f8b) << quint16(0x0800)
                 << quint32(origLastModified.toSecsSinceEpoch())
#if defined Q_OS_UNIX
                 << quint16(0x0003);
#elif defined Q_OS_WIN
                 << quint16(0x000b);
#elif defined Q_OS_MACOS
                 << quint16(0x0007);
#else
                 << quint16(0x00ff);
#endif

    // append a four-byte CRC-32 of the uncompressed data
    // append 4 bytes uncompressed input size modulo 2^32
    QByteArray footer;
    QDataStream footerStream(&footer, QIODevice::WriteOnly);
    footerStream << crc32buf(data) << quint32(data.size());

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

    QByteArray compressedData = qCompress(data, zlibCompressionLevel);
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

    // Strip the first six bytes (a 4-byte length put on by qCompress and a 2-byte zlib header)
    // and the last four bytes (a zlib integrity check).
    compressedData.remove(0, 6);
    compressedData.chop(4);

    if (Q_UNLIKELY(output.write(compressedData) < 0)) {
        qCCritical(C_STATICCOMPRESSED).nospace() << "Failed to write compressed deflate file "
                                                 << inputPath << ": " << output.errorString();
        return false;
    }

    return true;
}

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
bool StaticCompressedPrivate::compressZopfli(const QString &inputPath,
                                             const QString &outputPath) const
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
        input.close();
        return false;
    }

    ZopfliOptions options;
    ZopfliInitOptions(&options);
    options.numiterations = zopfliIterations;

    unsigned char *out{nullptr};
    size_t outSize{0};

    ZopfliCompress(&options,
                   ZopfliFormat::ZOPFLI_FORMAT_GZIP,
                   reinterpret_cast<const unsigned char *>(data.constData()),
                   data.size(),
                   &out,
                   &outSize);

    bool ok = false;
    if (outSize > 0) {
        QFile output(outputPath);
        if (Q_UNLIKELY(!output.open(QIODevice::WriteOnly))) {
            qCWarning(C_STATICCOMPRESSED)
                << "Can not open output file to compress with zopfli:" << outputPath;
        } else {
            if (Q_UNLIKELY(output.write(reinterpret_cast<const char *>(out), outSize) < 0)) {
                qCCritical(C_STATICCOMPRESSED).nospace()
                    << "Failed to write compressed zopfi file " << inputPath << ": "
                    << output.errorString();
                if (output.exists()) {
                    if (Q_UNLIKELY(!output.remove())) {
                        qCWarning(C_STATICCOMPRESSED)
                            << "Can not remove invalid compressed zopfli file:" << outputPath;
                    }
                }
            } else {
                ok = true;
            }
        }
    } else {
        qCWarning(C_STATICCOMPRESSED)
            << "Failed to compress file with zopfli, compressed data is empty:" << inputPath;
    }

    free(out);

    return ok;
}
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
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

    bool ok = false;

    size_t outSize = BrotliEncoderMaxCompressedSize(static_cast<size_t>(data.size()));
    if (Q_LIKELY(outSize > 0)) {
        const uint8_t *in = (const uint8_t *) data.constData();
        uint8_t *out      = (uint8_t *) malloc(sizeof(uint8_t) * (outSize + 1));
        if (Q_LIKELY(out != nullptr)) {
            BROTLI_BOOL status = BrotliEncoderCompress(brotliQualityLevel,
                                                       BROTLI_DEFAULT_WINDOW,
                                                       BROTLI_DEFAULT_MODE,
                                                       data.size(),
                                                       in,
                                                       &outSize,
                                                       out);
            if (Q_LIKELY(status == BROTLI_TRUE)) {
                QFile output(outputPath);
                if (Q_LIKELY(output.open(QIODevice::WriteOnly))) {
                    if (Q_LIKELY(output.write(reinterpret_cast<const char *>(out), outSize) > -1)) {
                        ok = true;
                    } else {
                        qCWarning(C_STATICCOMPRESSED).nospace()
                            << "Failed to write brotli compressed data to output file "
                            << outputPath << ": " << output.errorString();
                        if (output.exists()) {
                            if (Q_UNLIKELY(!output.remove())) {
                                qCWarning(C_STATICCOMPRESSED)
                                    << "Can not remove invalid compressed brotli file:"
                                    << outputPath;
                            }
                        }
                    }
                } else {
                    qCWarning(C_STATICCOMPRESSED)
                        << "Failed to open output file for brotli compression:" << outputPath;
                }
            } else {
                qCWarning(C_STATICCOMPRESSED) << "Failed to compress" << inputPath << "with brotli";
            }
            free(out);
        } else {
            qCWarning(C_STATICCOMPRESSED)
                << "Can not allocate needed output buffer of size"
                << (sizeof(uint8_t) * (outSize + 1)) << "for brotli compression.";
        }
    } else {
        qCWarning(C_STATICCOMPRESSED) << "Needed output buffer too large to compress input of size"
                                      << data.size() << "with brotli";
    }

    return ok;
}
#endif

#include "moc_staticcompressed.cpp"
