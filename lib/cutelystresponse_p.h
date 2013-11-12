/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYSTRESPONSE_P_H
#define CUTELYSTRESPONSE_P_H

#include "cutelystresponse.h"

#include <QMap>
#include <QUrl>

class CutelystEngine;
class CutelystResponsePrivate
{
public:
    CutelystResponsePrivate();

    quint16 status;
    quint16 finalizedHeaders;
    // We use a map since QHash *might*
    // cause issues in browsers due to
    // it's random ordering
    QMap<QString, QString> headers;
    QByteArray body;
    QUrl redirect;
    QString contentType;
    quint64 contentLength;
    CutelystEngine *engine;
};

#endif // CUTELYSTRESPONSE_P_H
