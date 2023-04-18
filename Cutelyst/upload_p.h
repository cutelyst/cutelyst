/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef UPLOAD_P_H
#define UPLOAD_P_H

#include "upload.h"

#include <QMultiHash>

namespace Cutelyst {

class UploadPrivate
{
public:
    inline UploadPrivate(QIODevice *dev, const Headers &hdrs, qint64 startOffst, qint64 endOffst)
        : headers(hdrs)
        , device(dev)
        , startOffset(startOffst)
        , endOffset(endOffst)
    {
    }

    Headers headers;
    QString name;
    QString filename;
    QIODevice *device;
    qint64 startOffset = 0;
    qint64 endOffset   = 0;
    qint64 pos         = 0;
};

} // namespace Cutelyst

#endif // UPLOAD_P_H
