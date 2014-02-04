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

#ifndef ENGINE_HTTP_P_H
#define ENGINE_HTTP_P_H

#include "enginehttp.h"
#include "childprocess.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

namespace Cutelyst {

class EngineHttpRequest : public QTcpSocket
{
    Q_OBJECT
public:
    explicit EngineHttpRequest(int socket, QObject *parent = 0);

    int connectionId();
    bool processing();
    void finish();

public slots:
    void process();
    void timeout();

Q_SIGNALS:
    void requestReady(void *requestData,
                      const QUrl &url,
                      const QByteArray &method,
                      const QByteArray &protocol,
                      const QHash<QByteArray, QByteArray> &headers,
                      const QByteArray &body);

private:
    bool m_finishedHeaders;
    bool m_processing;
    int m_connectionId;
    QVariantHash m_data;
    QByteArray m_buffer;
    QByteArray m_body;
    int m_bodySize;
    quint64 m_bufLastIndex;
    QByteArray m_method;
    QString m_path;
    QByteArray m_protocol;
    QTimer m_timeoutTimer;
    QHash<QByteArray, QByteArray> m_headers;
};

class EngineHttpPrivate
{
public:
    quint16 port = 3000;
    QHostAddress address = QHostAddress::Any;
    QTcpServer *server;
    QList<CutelystChildProcess*> child;
    QHash<int, EngineHttpRequest*> requests;
};

}

#endif // ENGINE_HTTP_P_H
