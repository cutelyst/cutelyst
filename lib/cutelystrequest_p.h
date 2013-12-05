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

#ifndef CUTELYSTREQUEST_P_H
#define CUTELYSTREQUEST_P_H

#include "cutelystrequest.h"

#include <QStringList>
#include <QHostAddress>
#include <QUrl>

namespace Cutelyst {

class CutelystEngine;
class CutelystRequestPrivate
{
public:
    QByteArray method;
    QUrl url;
    QStringList args;
    QString protocol;
    QList<QNetworkCookie> cookies;
    QHash<QString, QByteArray> headers;
    QByteArray body;
    QMultiHash<QString, QString> bodyParam;
    QMultiHash<QString, QString> queryParam;
    QMultiHash<QString, QString> param;
    const CutelystEngine *engine;
    int connectionId;
};

}

#endif // CUTELYSTREQUEST_P_H
