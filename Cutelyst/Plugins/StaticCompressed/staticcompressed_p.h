/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATICCOMPRESSED_P_H
#define STATICCOMPRESSED_P_H

#include "staticcompressed.h"

#include <QDir>
#include <QRegularExpression>
#include <QVector>

namespace Cutelyst {

class Context;

class StaticCompressedPrivate
{
public:
    enum Compression { Gzip, Zopfli, Brotli, Deflate };

    void beforePrepareAction(Context *c, bool *skipMethod);
    bool locateCompressedFile(Context *c, const QString &relPath) const;
    Q_REQUIRED_RESULT QString locateCacheFile(const QString &origPath,
                                              const QDateTime &origLastModified,
                                              Compression compression) const;
    Q_REQUIRED_RESULT bool compressGzip(const QString &inputPath,
                                        const QString &outputPath,
                                        const QDateTime &origLastModified) const;
    Q_REQUIRED_RESULT bool compressDeflate(const QString &inputPath,
                                           const QString &outputPath) const;
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
    Q_REQUIRED_RESULT bool compressZopfli(const QString &inputPath,
                                          const QString &outputPath) const;
#endif
#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    Q_REQUIRED_RESULT bool compressBrotli(const QString &inputPath,
                                          const QString &outputPath) const;
#endif

    QStringList dirs;
    QStringList mimeTypes;
    QStringList suffixes;
    QVector<QDir> includePaths;
    QRegularExpression re = QRegularExpression(QStringLiteral("\\.[^/]+$"));
    QDir cacheDir;
    constexpr static int zlibCompressionLevelDefault{9};
    constexpr static int zlibCompressionLevelMin{0};
    constexpr static int zlibCompressionLevelMax{9};
    int zlibCompressionLevel{zlibCompressionLevelDefault};
    constexpr static int zopfliIterationsDefault{15};
    constexpr static int zopfliIterationsMin{1};
    int zopfliIterations{zopfliIterationsDefault};
    constexpr static int brotliQualityLevelDefault{11};
    int brotliQualityLevel{brotliQualityLevelDefault};
    bool useZopfli{false};
    bool checkPreCompressed{true};
    bool onTheFlyCompression{true};
};

} // namespace Cutelyst

#endif // STATICCOMPRESSED_P_H
