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

#include "cutelystenginehttp_p.h"

#include "cutelyst.h"
#include "cutelystresponse.h"
#include "cutelystrequest_p.h"

#include <QCoreApplication>
#include <QStringList>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QHostInfo>
#include <QDateTime>
#include <QTcpSocket>
#include <QUrl>

CutelystEngineHttp::CutelystEngineHttp(QObject *parent) :
    CutelystEngine(parent),
    d_ptr(new CutelystEngineHttpPrivate)

{
    Q_D(CutelystEngineHttp);

    d->server = new QTcpServer(this);
    connect(d->server, &QTcpServer::newConnection,
            this, &CutelystEngineHttp::onNewServerConnection);

    QStringList args = QCoreApplication::arguments();
    for (int i = 0; i < args.count(); ++i) {
        QString argument = args.at(i);
        if (argument.startsWith(QLatin1String("--port=")) && argument.length() > 7) {
            d->port = argument.mid(7).toInt();
            qDebug() << Q_FUNC_INFO << "Using custom port:" << d->port;
        }
    }
}

CutelystEngineHttp::~CutelystEngineHttp()
{
    delete d_ptr;
}

bool CutelystEngineHttp::init()
{
    Q_D(CutelystEngineHttp);

    if (d->server->listen(d->address, d->port)) {
        int childCount = 1;
        for (int i = 0; i < childCount; ++i) {
            bool childProcess;
            CutelystChildProcess *child = new CutelystChildProcess(childProcess, this);
            if (childProcess) {
                // We are not the parent anymore,
                // so we don't need the server class
                connect(child, &CutelystChildProcess::newConnection,
                        this, &CutelystEngineHttp::onNewClientConnection);
                delete d->server;
                return true;
            } else if (child->initted()) {
                d->child << child;
            } else {
                delete child;
            }
        }
        qDebug() << "Listening on:" << d->server->serverAddress() << d->server->serverPort();
        qDebug() << "Number of child process:" << d->child.size();
    } else {
        qWarning() << "Failed to listen on" << d->address.toString() << d->port;
    }

    return !d->child.isEmpty();
}

quint16 CutelystEngineHttp::peerPort() const
{
    Q_D(const CutelystEngineHttp);
    return 0;//d->socket->peerPort();
}

QString CutelystEngineHttp::peerName() const
{
    Q_D(const CutelystEngineHttp);
    return QString();//d->socket->peerName();
}

QHostAddress CutelystEngineHttp::peerAddress() const
{
    Q_D(const CutelystEngineHttp);
    return QHostAddress();//d->socket->peerAddress();
}

void CutelystEngineHttp::finalizeHeaders(Cutelyst *c)
{
    Q_D(CutelystEngineHttp);

    QByteArray header;
    header.append(QString::fromLatin1("HTTP/1.1 %1\r\n").arg(statusString(c->response()->status())));

    QMap<QString, QString> headers = c->response()->headers();
    headers.insert(QLatin1String("Date"), QDateTime::currentDateTime().toString(Qt::ISODate));
    headers.insert(QLatin1String("Server"), QLatin1String("CutelystEngineHttp"));

    QMap<QString, QString>::ConstIterator it = headers.constBegin();
    while (it != headers.constEnd()) {
        header.append(it.key() % QLatin1String(": ") % it.value() % QLatin1String("\r\n"));
        ++it;
    }
    header.append(QLatin1String("\r\n"));

    d->requests[c->req()->connectionId()]->write(header);
}

void CutelystEngineHttp::finalizeBody(Cutelyst *c)
{
    Q_D(CutelystEngineHttp);

    CutelystEngineHttpRequest *req = d->requests.take(c->req()->connectionId());
    req->write(c->response()->body());
    req->waitForBytesWritten();
    req->close();
    req->deleteLater();
}

void CutelystEngineHttp::finalizeError(Cutelyst *c)
{

}

void CutelystEngineHttp::parse(const QByteArray &request)
{

}

QString CutelystEngineHttp::statusString(quint16 status) const
{
    QString ret;
    switch (status) {
    case CutelystResponse::OK:
        ret = QLatin1String("OK");
        break;
    case CutelystResponse::MovedPermanently:
        ret = QLatin1String("Moved Permanently");
        break;
    case CutelystResponse::Found:
        ret = QLatin1String("Found");
        break;
    case CutelystResponse::NotModified:
        ret = QLatin1String("Not Modified");
        break;
    case CutelystResponse::TemporaryRedirect:
        ret = QLatin1String("Temporary Redirect");
        break;
    case CutelystResponse::BadRequest:
        ret = QLatin1String("Bad Request");
        break;
    case CutelystResponse::AuthorizationRequired:
        ret = QLatin1String("Authorization Required");
        break;
    case CutelystResponse::Forbidden:
        ret = QLatin1String("Forbidden");
        break;
    case CutelystResponse::NotFound:
        ret = QLatin1String("Not Found");
        break;
    case CutelystResponse::MethodNotAllowed:
        ret = QLatin1String("Method Not Allowed");
        break;
    case CutelystResponse::InternalServerError:
        ret = QLatin1String("Internal Server Error");
        break;
    }

    if (ret.isEmpty()) {
        return QString::number(status);
    }
    return QString::number(status) % QLatin1Char(' ') % ret;
}

void CutelystEngineHttp::onNewServerConnection()
{
    Q_D(CutelystEngineHttp);

    QTcpSocket *socket = d->server->nextPendingConnection();
    if (socket) {
        if (!d->child.isEmpty()) {
            if (d->child.first()->sendFD(socket->socketDescriptor())) {
//                qDebug() << "fd sent";
            }
        }
        delete socket;
    }
}

void CutelystEngineHttp::onNewClientConnection(int socket)
{
    Q_D(CutelystEngineHttp);

    CutelystEngineHttpRequest *tcpSocket = new CutelystEngineHttpRequest(socket, this);
    if (tcpSocket->setSocketDescriptor(socket)) {
        d->requests.insert(socket, tcpSocket);
        connect(tcpSocket, &CutelystEngineHttpRequest::requestReady,
                this, &CutelystEngineHttp::createRequest);
    } else {
        delete tcpSocket;
    }
}

CutelystEngineHttpPrivate::CutelystEngineHttpPrivate() :
    port(3000),
    address(QHostAddress::Any)
{
}

CutelystEngineHttpRequest::CutelystEngineHttpRequest(int socket, QObject *parent) :
    QTcpSocket(parent),
    m_bufLastIndex(0),
    m_finishedHeaders(false)
{
    connect(this, &CutelystEngineHttpRequest::readyRead,
            this, &CutelystEngineHttpRequest::process);
}

void CutelystEngineHttpRequest::process()
{
//    qDebug() << Q_FUNC_INFO;

    m_buffer.append(readAll());

//    qDebug() << request;

    int newLine;
    if (m_method.isEmpty()) {
        if ((newLine = m_buffer.indexOf('\n', m_bufLastIndex)) != -1) {
            QByteArray section = m_buffer.mid(m_bufLastIndex, newLine - m_bufLastIndex - 1);
            m_bufLastIndex = newLine + 1;

            QRegularExpression methodProtocolRE("(\\w+)\\s+(.*)(?:\\s+(HTTP.*))$");
            QRegularExpression methodRE("(\\w+)\\s+(.*)");
            bool badRequest = false;
            QRegularExpressionMatch match = methodProtocolRE.match(section);
            if (match.hasMatch()) {
                m_method = match.captured(1).toLocal8Bit();
                m_path = match.captured(2);
                m_protocol = match.captured(3);
            } else {
                match = methodRE.match(section);
                if (match.hasMatch()) {
                    m_method = match.captured(1).toLocal8Bit();
                    m_path = match.captured(2);
                }
            }

            if (badRequest) {
                qDebug() << "BAD REQUEST" << m_buffer;
                return;
            }
        }
    }

    if (!m_finishedHeaders) {
        while ((newLine = m_buffer.indexOf('\n', m_bufLastIndex)) != -1) {
            QString section = m_buffer.mid(m_bufLastIndex, newLine - m_bufLastIndex - 1);
//            qDebug() << "[header] " << section << section.isEmpty();
            m_bufLastIndex = newLine + 1;

            if (!section.isEmpty()) {
                m_headers[section.section(QLatin1Char(':'), 0, 0)] = section.section(QLatin1Char(':'), 1).trimmed().toUtf8();
            } else {
                m_bodySize = m_headers.value(QLatin1String("Content-Length")).toULongLong();
                m_finishedHeaders = true;
            }
        }
    }

    if (!m_finishedHeaders) {
        return;
    }

    m_body = m_buffer.mid(m_bufLastIndex, m_bodySize);
//    qDebug() << "m_bodySize " << m_bodySize << m_body.size() << m_body;
    if (m_bodySize != m_body.size()) {
        return;
    }

    QUrl url;
    if (m_headers.contains(QLatin1String("Host"))) {
        url = QLatin1String("http://") % m_headers[QLatin1String("Host")] % m_path;
    } else {
        url = QLatin1String("http://") % QHostInfo::localHostName() % m_path;
    }

    requestReady(socketDescriptor(),
                 url,
                 m_method,
                 m_protocol,
                 m_headers,
                 m_body);

    m_buffer.clear();
    m_body.clear();
    m_headers.clear();
    m_method.clear();
    m_protocol.clear();
    m_bufLastIndex = 0;
    m_finishedHeaders = false;
}
