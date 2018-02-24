/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef STATICCOMPRESSED_P_H
#define STATICCOMPRESSED_P_H

#include "staticcompressed.h"

#include <QRegularExpression>
#include <QVector>
#include <QDir>

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
    int zopfliIterations = 15;
    int brotliQualityLevel = 11;
    bool useZopfli = false;
    bool checkPreCompressed = true;
    bool onTheFlyCompression = true;
};

}

#endif // STATICCOMPRESSED_P_H
