/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include "cwsgiengine.h"

#include "protocolhttp.h"
#include "tcpserver.h"
#include "config.h"
#include "wsgi.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Request>
#include <Cutelyst/Application>

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QElapsedTimer>
#include <QLocalServer>
#include <QLocalSocket>

using namespace CWSGI;

QByteArray dateHeader();

CWsgiEngine::CWsgiEngine(Application *app, int workerCore, const QVariantMap &opts, WSGI *wsgi) : Engine(app, workerCore, opts)
  , m_wsgi(wsgi)
{
    m_serverHeader.append("\r\nServer: cutelyst/").append(VERSION).append("\r\n\r\n");
    m_lastDate = dateHeader();
    m_lastDateTimer.start();
}

int CWsgiEngine::workerId() const
{
    return m_workerId;
}

void CWsgiEngine::setTcpSockets(const std::vector<QTcpServer *> &sockets)
{
    m_sockets = sockets;
}

void CWsgiEngine::listen()
{
    if (workerCore() > 0) {
        // init and postfork
        if (!initApplication()) {
            qCritical() << "Failed to init application on a different thread than main. Are you sure threaded mode is supported in this application?";
            return;
        }

    }

    const auto sockets = m_sockets;
    for (QTcpServer *socket : sockets) {
        const QString serverAddress = socket->serverAddress().toString() + QLatin1Char(':') + QString::number(socket->serverPort());
        auto server = new TcpServer(serverAddress, m_wsgi, this);
        server->setSocketDescriptor(socket->socketDescriptor());
        server->pauseAccepting();
        connect(this, &CWsgiEngine::resumeAccepting, server, &TcpServer::resumeAccepting);
    }

    Q_EMIT initted();
}

void CWsgiEngine::postFork()
{
    if (!postForkApplication()) {
        // CHEAP
        QCoreApplication::exit(15);
    }

    Q_EMIT resumeAccepting();
}

QByteArray dateHeader()
{
    QString ret;
    ret = QLatin1String("\r\nDate: ") + QLocale::c().toString(QDateTime::currentDateTimeUtc(),
                                                              QStringLiteral("ddd, dd MMM yyyy hh:mm:ss 'GMT"));
    return ret.toLatin1();
}

bool CWsgiEngine::finalizeHeadersWrite(Context *c, quint16 status, const Headers &headers, void *engineData)
{
    auto conn = static_cast<QIODevice*>(engineData);

    int msgLen;
    const char *msg = httpStatusMessage(status, &msgLen);
    conn->write(msg, msgLen);

    auto sock = qobject_cast<TcpSocket*>(conn);
    const auto headersData = headers.data();
    if (sock->headerClose == 1) {
        sock->headerClose = 0;
    }

    bool hasDate = false;
    auto it = headersData.constBegin();
    const auto endIt = headersData.constEnd();
    while (it != endIt) {
        const QString key = it.key();
        const QString value = it.value();
        if (sock->headerClose == 0 && key == QLatin1String("connection")) {
            if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
                sock->headerClose = 2;
            } else {
                sock->headerClose = 1;
            }
        } else if (!hasDate && key == QLatin1String("date")) {
            hasDate = true;
        }

        QString ret(QLatin1String("\r\n") + camelCaseHeader(key) + QLatin1String(": ") + value);
        conn->write(ret.toLatin1());

        ++it;
    }

    if (!hasDate) {
        if (m_lastDateTimer.hasExpired(1000)) {
            m_lastDate = dateHeader();
            m_lastDateTimer.restart();
        }
        conn->write(m_lastDate);
    }

    return conn->write(m_serverHeader) == m_serverHeader.size();
}

qint64 CWsgiEngine::doWrite(Context *c, const char *data, qint64 len, void *engineData)
{
    auto conn = static_cast<QIODevice*>(engineData);
    //    qDebug() << Q_FUNC_INFO << QByteArray(data,len);
    qint64 ret = conn->write(data, len);
    //    conn->waitForBytesWritten(200);
    return ret;
}

bool CWsgiEngine::init()
{
    return initApplication();
}

#include "moc_cwsgiengine.cpp"
