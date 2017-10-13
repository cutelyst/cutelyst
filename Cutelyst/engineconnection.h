/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef ENGINECONNECTION_H
#define ENGINECONNECTION_H

#include <QObject>
#include <QHostAddress>

#include <Cutelyst/Headers>

namespace Cutelyst {

class EngineConnection : public QObject
{
    Q_OBJECT
public:
    explicit EngineConnection(QObject *parent = nullptr);

public:
    /** The method used (GET, POST...) */
    QString method;
    /** The path requested by the user agent '/index' */
    QString path;
    /** The query string requested by the user agent 'foo=bar&baz' */
    QByteArray query;
    /** The protocol requested by the user agent 'HTTP1/1' */
    QString protocol;
    /** The server address which the server is listening to,
     *  usually the 'Host' header but if that's not present should be filled with the server address */
    QString serverAddress;
    /** The remote/client address */
    QHostAddress remoteAddress;
    /** The remote user name set by a front web server */
    QString remoteUser;
    /** The request headers */
    Headers headers;
    /** The timestamp of the start of headers */
    quint64 startOfRequest;
    /** The QIODevice containing the body (if any) of the request */
    QIODevice *body;
    /** The internal pointer of the request, to be used for mapping this request to the real request */
    void *requestPtr;
    /** The remote/client port */
    quint16 remotePort;
    /** If the connection is secure HTTPS */
    bool isSecure;
};

}

#endif // ENGINECONNECTION_H
