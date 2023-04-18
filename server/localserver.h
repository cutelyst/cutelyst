/*
 * SPDX-FileCopyrightText: (C) 2017-2019 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QLocalServer>

class QSocketNotifier;
namespace Cutelyst {

class Server;
class Protocol;
class LocalSocket;
class CWsgiEngine;
class LocalServer final : public QLocalServer
{
    Q_OBJECT
public:
    explicit LocalServer(Server *wsgi, QObject *parent = nullptr);

    void setProtocol(Protocol *protocol);

    LocalServer *createServer(CWsgiEngine *engine) const;

    void pauseAccepting();
    void resumeAccepting();

    virtual void incomingConnection(quintptr handle) override;

    qintptr socket() const;

    void shutdown();
    void timeoutConnections();

    Protocol *protocol() const;

private:
    QSocketNotifier *socketDescriptorNotifier() const;
#ifdef Q_OS_UNIX
    void socketNotifierActivated();
#endif

    QSocketNotifier *m_socketNotifier = nullptr;
    CWsgiEngine *m_engine             = nullptr;
    Server *m_wsgi;

    Protocol *m_protocol = nullptr;
    qintptr m_socket     = -1;
    int m_processing     = 0;
};

} // namespace Cutelyst

#endif // LOCALSERVER_H
