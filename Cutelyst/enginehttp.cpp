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

#include "enginehttp_p.h"

#include "context.h"
#include "response.h"
#include "request_p.h"

#include <QCoreApplication>
#include <QStringList>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QHostInfo>
#include <QDateTime>
#include <QTcpSocket>
#include <QMimeDatabase>
#include <QUrl>

using namespace Cutelyst;

CutelystEngineHttp::CutelystEngineHttp(QObject *parent) :
    Engine(parent),
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

void CutelystEngineHttp::finalizeHeaders(Context *ctx)
{
    Q_D(CutelystEngineHttp);

    QByteArray header;
    header.append(QString::fromLatin1("HTTP/1.1 %1\r\n").arg(statusCode(ctx->response()->status()).data()));

    QMap<QByteArray, QByteArray> headers = ctx->response()->headers();

    QDateTime utc = QDateTime::currentDateTime();
    utc.setTimeSpec(Qt::UTC);
    QString date = utc.toString(QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
    headers.insert("Date", date.toLocal8Bit());
    headers.insert("Server", "Cutelyst-HTTP-Engine");
    headers.insert("Connection", "keep-alive");

    QMap<QByteArray, QByteArray>::ConstIterator it = headers.constBegin();
    while (it != headers.constEnd()) {
        header.append(it.key() % QLatin1String(": ") % it.value() % QLatin1String("\r\n"));
        ++it;
    }
    header.append(QLatin1String("\r\n"));

    if (!headers.contains("Content-Type") &&
            !ctx->res()->body().isEmpty()) {
        QMimeDatabase db;
        QMimeType mimeType = db.mimeTypeForData(ctx->res()->body());
        if (mimeType.isValid()) {
            if (mimeType.name() == QLatin1String("text/html")) {
                headers.insert("Content-Type", "text/html; charset=utf-8");
            } else {
                headers.insert("Content-Type", mimeType.name().toLocal8Bit());
            }
        }
    }

    int *id = static_cast<int*>(ctx->req()->connectionId());
    d->requests[*id]->write(header);
}

void CutelystEngineHttp::finalizeBody(Context *ctx)
{
    Q_D(CutelystEngineHttp);

    int *id = static_cast<int*>(ctx->req()->connectionId());
    CutelystEngineHttpRequest *req = d->requests.value(*id);
    req->write(ctx->response()->body());
    req->finish();
}

void CutelystEngineHttp::removeConnection()
{
    Q_D(CutelystEngineHttp);
    CutelystEngineHttpRequest *req = static_cast<CutelystEngineHttpRequest*>(sender());
    if (req) {
        d->requests.take(req->connectionId());
    }
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
        connect(tcpSocket, &CutelystEngineHttpRequest::destroyed,
                this, &CutelystEngineHttp::removeConnection);
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
    m_finishedHeaders(false),
    m_processing(false),
    m_connectionId(socket),
    m_bufLastIndex(0)
{
    connect(this, &CutelystEngineHttpRequest::readyRead,
            this, &CutelystEngineHttpRequest::process);
    connect(this, &CutelystEngineHttpRequest::bytesWritten,
            this, &CutelystEngineHttpRequest::timeout);
    connect(&m_timeoutTimer, &QTimer::timeout,
            this, &CutelystEngineHttpRequest::timeout);
    m_timeoutTimer.setInterval(5000);
    m_timeoutTimer.setSingleShot(true);
    m_timeoutTimer.start();
}

int CutelystEngineHttpRequest::connectionId()
{
    return m_connectionId;
}

bool CutelystEngineHttpRequest::processing()
{
    return m_processing;
}

void CutelystEngineHttpRequest::finish()
{
    m_processing = false;
    if (!m_buffer.isNull() && bytesAvailable() == 0) {
        QTimer::singleShot(0, this, SLOT(process()));
    } else {
        m_timeoutTimer.start();
    }
}

void CutelystEngineHttpRequest::process()
{
//    qDebug() << Q_FUNC_INFO << connectionId() << m_processing << m_buffer.size() << sender() << bytesAvailable();
    m_buffer.append(readAll());

    if (m_processing) {
        return;
    }

    m_timeoutTimer.start();

//    qDebug() << m_buffer;

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
                m_headers[section.section(QLatin1Char(':'), 0, 0).toUtf8()] = section.section(QLatin1Char(':'), 1).trimmed().toUtf8();
            } else {
                m_bodySize = m_headers.value("Content-Length").toULongLong();
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
    if (m_headers.contains("Host")) {
        url = QLatin1String("http://") % m_headers["Host"] % m_path;
    } else {
        url = QLatin1String("http://") % QHostInfo::localHostName() % m_path;
    }

    m_buffer.clear();
    m_bufLastIndex = 0;
    m_finishedHeaders = false;
    m_processing = true;
    requestReady(&m_connectionId,
                 url,
                 m_method,
                 m_protocol,
                 m_headers,
                 m_body);

    m_body.clear();
    m_headers.clear();
    m_method.clear();
    m_protocol.clear();
}

void CutelystEngineHttpRequest::timeout()
{
    QTimer *timer = qobject_cast<QTimer*>(sender());
    if (timer && bytesToWrite() == 0 && bytesAvailable() == 0) {
        close();
        deleteLater();
    } else {
        m_timeoutTimer.start();
    }
}
