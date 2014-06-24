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

#ifndef ENGINE_HTTP_H
#define ENGINE_HTTP_H

#include "engine.h"

#include <QStringList>

namespace Cutelyst {

class EngineHttpPrivate;
class EngineHttp : public Engine
{
    Q_OBJECT
public:
    explicit EngineHttp(QObject *parent = 0);
    ~EngineHttp();

    bool init();

    virtual void finalizeHeaders(Context *ctx);
    virtual void finalizeBody(Context *ctx);

protected:
    EngineHttpPrivate *d_ptr;

private Q_SLOTS:
    void removeConnection();
    void processRequest(void *requestData,
                        const QUrl &url,
                        const QByteArray &method,
                        const QByteArray &protocol,
                        const QHash<QByteArray, QByteArray> &headers,
                        QIODevice *body);

private:
    Q_DECLARE_PRIVATE(EngineHttp)

    void onNewServerConnection();
    void onNewClientConnection(int socket);
};

}

#endif // ENGINE_HTTP_H
