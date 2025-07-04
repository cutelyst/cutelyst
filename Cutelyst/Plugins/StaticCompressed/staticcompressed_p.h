/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATICCOMPRESSED_P_H
#define STATICCOMPRESSED_P_H

#include "staticcompressed.h"

#include <QDir>
#include <QRegularExpression>
#include <QVector>

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
#    include <zopfli.h>
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
#    include <zstd.h>
#endif

using namespace Qt::StringLiterals;

namespace Cutelyst {

class Context;

class StaticCompressedPrivate
{
public:
    enum Compression { Gzip, ZopfliGzip, Brotli, Deflate, ZopfliDeflate, Zstd };

    void beforePrepareAction(Context *c, bool *skipMethod);
    bool locateCompressedFile(Context *c, const QString &relPath) const;
    [[nodiscard]] QString locateCacheFile(const QString &origPath,
                                          const QDateTime &origLastModified,
                                          Compression compression) const;

    void loadZlibConfig(const QVariantMap &conf);

    [[nodiscard]] bool compressGzip(const QString &inputPath,
                                    const QString &outputPath,
                                    const QDateTime &origLastModified) const;

    [[nodiscard]] bool compressDeflate(const QString &inputPath, const QString &outputPath) const;

    struct ZlibConfig {
        constexpr static int compressionLevelDefault{9};
        constexpr static int compressionLevelMin{0};
        constexpr static int compressionLevelMax{9};
        int compressionLevel{compressionLevelDefault};
    } zlib;

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
    void loadZopfliConfig(const QVariantMap &conf);

    [[nodiscard]] bool compressZopfli(const QString &inputPath,
                                      const QString &outputPath,
                                      ZopfliFormat format) const;

    struct ZopfliConfig {
        constexpr static int iterationsDefault{15};
        constexpr static int iterationsMin{1};
        ZopfliOptions options;
    } zopfli;
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    void loadBrotliConfig(const QVariantMap &conf);

    [[nodiscard]] bool compressBrotli(const QString &inputPath, const QString &outputPath) const;

    struct BrotliConfig {
        constexpr static int qualityLevelDefault{11};
        int qualityLevel{qualityLevelDefault};
    } brotli;
#endif

#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZSTD
    [[nodiscard]] bool loadZstdConfig(const QVariantMap &conf);

    [[nodiscard]] bool compressZstd(const QString &inputPath, const QString &outputPath) const;

    struct ZstdConfig {
        ~ZstdConfig() { ZSTD_freeCCtx(ctx); }

        ZSTD_CCtx *ctx{nullptr};
        constexpr static int compressionLevelDefault{9};
        int compressionLevel{compressionLevelDefault};
    } zstd;
#endif

    QStringList dirs;
    QStringList mimeTypes;
    QStringList suffixes;
    QStringList compressionFormatOrder;
    QVector<QDir> includePaths;
    QVariantMap defaultConfig;
    QRegularExpression re = QRegularExpression(u"\\.[^/]+$"_s);
    QDir cacheDir;
    bool useZopfli{false};
    bool checkPreCompressed{true};
    bool onTheFlyCompression{true};
    bool serveDirsOnly{false};
};

} // namespace Cutelyst

#endif // STATICCOMPRESSED_P_H
