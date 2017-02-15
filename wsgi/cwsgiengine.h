/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CWSGI_ENGINE_H
#define CWSGI_ENGINE_H

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>
#include <Cutelyst/Engine>

class QTcpServer;

namespace CWSGI {

class Protocol;
struct SocketInfo {
    QString serverName;
    Protocol *protocol;
    bool localSocket;
    qintptr socketDescriptor = 0;
};

class WSGI;
class CWsgiEngine : public Cutelyst::Engine
{
    Q_OBJECT
public:
    CWsgiEngine(Cutelyst::Application *app, int workerCore, const QVariantMap &opts, WSGI *wsgi);

    virtual int workerId() const;

    inline void processSocket(Cutelyst::EngineRequest *sock) {
        processRequest(*sock);
    }

    void setTcpSockets(const std::vector<SocketInfo> &sockets);

    void listen();

    void postFork();

    int m_workerId = 0;

    virtual bool init();

Q_SIGNALS:
    void initted();
    void started();
    void shutdown();
    void shutdownCompleted(CWsgiEngine *engine);

protected:
    virtual bool finalizeHeadersWrite(Cutelyst::Context *c, quint16 status,  const Cutelyst::Headers &headers, void *engineData);

    virtual qint64 doWrite(Cutelyst::Context *c, const char *data, qint64 len, void *engineData);

    inline void startSocketTimeout() {
        if (m_socketTimeout && ++m_serversTimeout == 1) {
            m_socketTimeout->start();
        }
    }

    inline void stopSocketTimeout() {
        if (m_socketTimeout && --m_serversTimeout == 0) {
            m_socketTimeout->stop();
        }
    }

    inline void serverShutdown() {
        if (--m_servers == 0) {
            Q_EMIT shutdownCompleted(this);
        }
    }

private:
    friend class ProtocolHttp;
    friend class ProtocolFastCGI;
    friend class LocalServer;
    friend class TcpServer;

    std::vector<SocketInfo> m_sockets;
    QByteArray m_lastDate;
    QElapsedTimer m_lastDateTimer;
    QTimer *m_socketTimeout = nullptr;
    WSGI *m_wsgi;
    int m_servers = 0;
    int m_serversTimeout = 0;
};

}

#endif // CWSGI_ENGINE_H
