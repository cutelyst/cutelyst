/*
 * Copyright (C) 2017-2019 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QLocalServer>

class QSocketNotifier;
namespace CWSGI {

class WSGI;
class Protocol;
class LocalSocket;
class CWsgiEngine;
class LocalServer : public QLocalServer
{
    Q_OBJECT
public:
    explicit LocalServer(WSGI *wsgi, QObject *parent = nullptr);

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
    CWsgiEngine *m_engine = nullptr;
    WSGI *m_wsgi;

    std::vector<LocalSocket *> m_socks;
    Protocol *m_protocol = nullptr;
    qintptr m_socket = -1;
    int m_processing = 0;
};

}

#endif // LOCALSERVER_H
