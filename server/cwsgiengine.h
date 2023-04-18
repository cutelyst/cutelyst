/*
 * SPDX-FileCopyrightText: (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CWSGI_ENGINE_H
#define CWSGI_ENGINE_H

#include <Cutelyst/Engine>

#include <QElapsedTimer>
#include <QObject>
#include <QTimer>

class QTcpServer;

namespace Cutelyst {

class TcpServer;
class Protocol;
class ProtocolFastCGI;
class ProtocolHttp;
class ProtocolHttp2;
class Server;
class Socket;
class CWsgiEngine final : public Cutelyst::Engine
{
    Q_OBJECT
public:
    CWsgiEngine(Cutelyst::Application *localApp, int workerCore, const QVariantMap &opts, Server *wsgi);
    virtual ~CWsgiEngine() override;

    virtual int workerId() const override;

    void setServers(const std::vector<QObject *> &servers);

    void postFork(int workerId);

    int m_workerId = 0;

    virtual bool init() override;

    inline QByteArray lastDate()
    {
        if (m_lastDateTimer.hasExpired(1000)) {
            m_lastDate = dateHeader();
            m_lastDateTimer.restart();
        }
        return m_lastDate;
    }

    void handleSocketShutdown(Socket *sock);

Q_SIGNALS:
    void started();
    void shutdown();
    void shutdownCompleted(Cutelyst::CWsgiEngine *engine);

protected:
    inline void startSocketTimeout()
    {
        if (m_socketTimeout && ++m_serversTimeout == 1) {
            m_socketTimeout->start();
        }
    }

    inline void stopSocketTimeout()
    {
        if (m_socketTimeout && --m_serversTimeout == 0) {
            m_socketTimeout->stop();
        }
    }

    inline void serverShutdown()
    {
        if (--m_runningServers == 0) {
            Q_EMIT shutdownCompleted(this);
        }
    }

    static QByteArray dateHeader();

private:
    friend class ProtocolHttp;
    friend class ProtocolFastCGI;
    friend class LocalServer;
    friend class TcpServer;
    friend class TcpSslServer;
    friend class Connection;
    friend class Socket;

    Protocol *getProtoHttp();
    ProtocolHttp2 *getProtoHttp2();
    Protocol *getProtoFastCgi();

    QByteArray m_lastDate;
    QElapsedTimer m_lastDateTimer;
    QTimer *m_socketTimeout = nullptr;
    Server *m_wsgi;
    ProtocolHttp *m_protoHttp    = nullptr;
    ProtocolHttp2 *m_protoHttp2  = nullptr;
    ProtocolFastCGI *m_protoFcgi = nullptr;
    int m_runningServers         = 0;
    int m_serversTimeout         = 0;
};

} // namespace Cutelyst

#endif // CWSGI_ENGINE_H
