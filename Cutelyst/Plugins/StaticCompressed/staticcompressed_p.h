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
    enum Compression {
        Gzip,
        Zopfli,
        Brotli,
        Deflate
    };

    void beforePrepareAction(Context *c, bool *skipMethod);
    bool locateCompressedFile(Context *c, const QString &relPath) const;
    QString locateCacheFile(const QString &origPath, const QDateTime &origLastModified, Compression compression) const;
    bool compressGzip(const QString &inputPath, const QString &outputPath, const QDateTime &origLastModified) const;
    bool compressDeflate(const QString &inputPath, const QString &outputPath) const;
#ifdef CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI
    bool compressZopfli(const QString &inputPath, const QString &outputPath) const;
#endif
#ifdef CUTELYST_STATICCOMPRESSED_WITH_BROTLI
    bool compressBrotli(const QString &inputPath, const QString &outputPath) const;
#endif

    QStringList dirs;
    QStringList mimeTypes;
    QStringList suffixes;
    QVector<QDir> includePaths;
    QRegularExpression re = QRegularExpression(QStringLiteral("\\.[^/]+$"));
    QDir cacheDir;
    int zlibCompressionLevel = 9;
    int zopfliIterations     = 15;
    int brotliQualityLevel   = 11;
    bool useZopfli           = false;
    bool checkPreCompressed  = true;
    bool onTheFlyCompression = true;
};

} // namespace Cutelyst

#endif // STATICCOMPRESSED_P_H
