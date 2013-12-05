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

#ifndef CUTELYST_ENGINE_H
#define CUTELYST_ENGINE_H

#include <QObject>
#include <QHostAddress>

namespace Cutelyst {

class Context;
class Request;
class Response;
class EnginePrivate;
class Engine : public QObject
{
    Q_OBJECT
public:
    explicit Engine(QObject *parent = 0);
    ~Engine();

    virtual quint16 peerPort() const = 0;
    virtual QString peerName() const = 0;
    virtual QHostAddress peerAddress() const = 0;

    virtual bool init() = 0;

    void finalizeCookies(Context *ctx);
    virtual void finalizeHeaders(Context *ctx) = 0;
    virtual void finalizeBody(Context *ctx) = 0;
    void finalizeError(Context *ctx);

Q_SIGNALS:
    void handleRequest(Request *request, Response *response);

protected Q_SLOTS:
    void createRequest(int connectionId,
                       const QUrl &url,
                       const QByteArray &method,
                       const QString &protocol,
                       const QHash<QString, QByteArray> &headers,
                       const QByteArray &body);
protected:
    EnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Engine)
};

}

#endif // CUTELYST_ENGINE_H
