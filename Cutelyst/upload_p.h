/*
 * Copyright (C) 2014-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef UPLOAD_P_H
#define UPLOAD_P_H

#include "upload.h"

#include <QMultiHash>

namespace Cutelyst {

class UploadPrivate
{
public:
    inline UploadPrivate(QIODevice *dev, const Headers &hdrs, qint64 startOffst, qint64 endOffst) : headers(hdrs)
      , device(dev)
      , startOffset(startOffst)
      , endOffset(endOffst)
    { }

    Headers headers;
    QString name;
    QString filename;
    QIODevice *device;
    qint64 startOffset = 0;
    qint64 endOffset = 0;
    qint64 pos = 0;
};

}

#endif // UPLOAD_P_H
