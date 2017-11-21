/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef STATICCOMPRESSED_P_H
#define STATICCOMPRESSED_P_H

#include "staticcompressed.h"

#include <QRegularExpression>
#include <QVector>
#include <QDir>
#include <QThread>

namespace Cutelyst {

class Context;

class StaticCompressedPrivate
{
public:
    QVector<QDir> includePaths;
    QStringList dirs;
    QRegularExpression re = QRegularExpression(QStringLiteral("\\.[^/]+$"));
    QDir cacheDir;
    int gzipCompressionLevel = -1;
    QStringList mimeTypes;
    QStringList suffixes;
    bool useZopfli = true;

    enum Compression {
        Gzip,
        Zopfli,
        Brotli
    };

    void beforePrepareAction(Context *c, bool *skipMethod);
    bool locateCompressedFile(Context *c, const QString &relPath) const;
    QString locateCacheFile(const QString &origPath, const QDateTime &origLastModified, Compression compression) const;
    bool compressGzip(const QString &inputPath, const QString &outputPath, const QDateTime &origLastModified) const;
#ifdef ZOPFLI_ENABLED
    bool compressZopfli(const QString &inputPath, const QString &outputPath) const;
#endif
#ifdef BROTLI_ENABLED
    bool compressBrotli(const QString &inputPath, const QString &outputPath) const;
#endif
};

}

#endif // STATICCOMPRESSED_P_H
