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

#include <Cutelyst/context.h>
#include <Cutelyst/response.h>
#include <Cutelyst/request_p.h>
#include <Cutelyst/application.h>
#include <Cutelyst/common.h>

#include <QCoreApplication>
#include <QStringList>
#include <QRegularExpression>
#include <QStringBuilder>
#include <QHostInfo>
#include <QDateTime>
#include <QTcpSocket>
#include <QMimeDatabase>
#include <QUrl>
#include <QBuffer>
#include <QLoggingCategory>

#include <QCommandLineParser>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(CUTELYST_ENGINE_HTTP, "cutelyst.engine.http")

void cuteOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "%s[debug] %s\n", context.category, localMsg.constData());
        break;
    case QtWarningMsg:
        fprintf(stderr, "%s[warn] %s\n", context.category, localMsg.constData());
        break;
    case QtCriticalMsg:
        fprintf(stderr, "%s[crit] %s\n", context.category, localMsg.constData());
        break;
    case QtFatalMsg:
        fprintf(stderr, "%s[fatal] %s\n", context.category, localMsg.constData());
        abort();
    case QtInfoMsg:
        fprintf(stderr, "%s[info] %s\n", context.category, localMsg.constData());
        break;
    }
}

EngineHttp::EngineHttp(const QVariantMap &opts, QObject *parent) : Engine(opts, parent)
  , d_ptr(new EngineHttpPrivate)
{
    Q_D(EngineHttp);

    qInstallMessageHandler(cuteOutput);

//    d->server = new QTcpServer(this);
//    connect(d->server, &QTcpServer::newConnection,
//            this, &EngineHttp::onNewServerConnection);

    QStringList args = QCoreApplication::arguments();
    for (int i = 0; i < args.count(); ++i) {
        QString argument = args.at(i);
        if (argument.startsWith(QLatin1String("--port=")) && argument.length() > 7) {
            d->port = argument.midRef(7).toInt();
            qCDebug(CUTELYST_ENGINE_HTTP) << "Using custom port:" << d->port;
        }
    }

    QCommandLineParser parser;

    QCommandLineOption httpSocket(QStringLiteral("http-socket"),
                                  QCoreApplication::translate("main", "Bind to the specified UNIX/TCP socket using HTTP protocol."),
                                  QStringLiteral("address:port"));
    parser.addOption(httpSocket);

    // An option with a value
    QCommandLineOption workers(QStringList() << QStringLiteral("p") << QStringLiteral("processes") << QStringLiteral("workers-directory"),
                               QCoreApplication::translate("main", "Copy all source files into <directory>."), QStringLiteral("number"));
    parser.addOption(workers);

    // Process the actual command line arguments given by the user
    parser.process(*qApp);

    if (parser.isSet(workers)) {
        d->workers = parser.value(workers).toInt();
    }

    if (parser.isSet(httpSocket)) {
        Q_FOREACH (const QString &listen, parser.values(httpSocket)) {
            qCDebug(CUTELYST_ENGINE_HTTP) << "http-socket"<< listen;
            QTcpServer *server = new QTcpServer(this);
            QStringList parts = listen.split(QLatin1Char(':'));
            if (parts.size() != 2) {
                qCDebug(CUTELYST_ENGINE_HTTP) << "error parsing:" << listen;
                exit(1);
            }

            QHostAddress address;
            if (parts.first().isEmpty()) {
                address = QHostAddress::Any;
            } else {
                address.setAddress(parts.first());
            }

            if (server->listen(address, parts.last().toInt())) {
                qCDebug(CUTELYST_ENGINE_HTTP) << "Listening on:" << server->serverAddress() << server->serverPort();
            } else {
                qCWarning(CUTELYST_ENGINE_HTTP) << "Failed to listen on" << d->address.toString() << d->port;
                exit(1);
            }

            d->servers.append(server);
        }
    }

    if (d->servers.isEmpty()) {
        qCWarning(CUTELYST_ENGINE_HTTP) << "Not listening on anywhere";
        parser.showHelp(1);
    }
}

EngineHttp::~EngineHttp()
{
    delete d_ptr;
}

bool EngineHttp::init()
{
    Q_D(EngineHttp);

    for (int i = 0; i < d->workers; ++i) {
        bool childProcess;
        CutelystChildProcess *child = new CutelystChildProcess(childProcess, this);
        if (childProcess) {
            // We are not the parent anymore,
            // so we don't need the server class
            if (postForkApplication()) {
                //                    connect(child, &CutelystChildProcess::newConnection,
                //                            this, &EngineHttp::onNewClientConnection);
                //                    delete d->server;
                Q_FOREACH (QTcpServer *server, d->servers) {
                    connect(server, &QTcpServer::newConnection,
                            this, &EngineHttp::onNewServerConnection);
                }
                return true;
            } else {
                qCDebug(CUTELYST_ENGINE_HTTP) << "Failed to post fork";
                exit(1);
            }
        } else if (child->initted()) {
            d->child << child;
        } else {
            delete child;
        }
    }

    Q_FOREACH (QTcpServer *server, d->servers) {
        server->pauseAccepting();
    }

    qCDebug(CUTELYST_ENGINE_HTTP) << "Number of child process:" << d->child.size();

    return !d->child.isEmpty();
}

bool EngineHttp::finalizeHeaders(Context *ctx)
{
    Q_D(EngineHttp);

    QByteArray header;
    header.append(QStringLiteral("HTTP/1.1 %1\r\n")
                  .arg(QString::fromLatin1(statusCode(ctx->response()->status()))).toLatin1());

    Headers headers = ctx->response()->headers();

    QDateTime utc = QDateTime::currentDateTime();
    utc.setTimeSpec(Qt::UTC);
    headers.setDateWithDateTime(utc);
    headers.setServer(QStringLiteral("Cutelyst-HTTP-Engine"));
    headers.setHeader(QStringLiteral("Connection"), QStringLiteral("keep-alive"));
    headers.setContentLength(ctx->res()->contentLength());

    Q_FOREACH (const HeaderValuePair &pair, headersForResponse(headers)) {
        header.append(pair.key.toLatin1() + ": " + pair.value.toLatin1() + "\r\n");
    }
    header.append("\r\n");

    if (!headers.contains(QStringLiteral("Content-Type")) &&
            ctx->res()->hasBody()) {
        QMimeDatabase db;
        QMimeType mimeType = db.mimeTypeForData(ctx->res()->bodyDevice());
        if (mimeType.isValid()) {
            if (mimeType.name() == QLatin1String("text/html")) {
                headers.setContentType(QStringLiteral("text/html; charset=utf-8"));
            } else {
                headers.setContentType(mimeType.name());
            }
        }
    }

    int *id = static_cast<int*>(ctx->engineData());
    d->requests[*id]->m_socket->write(header);

    return true;
}

void EngineHttp::finalizeBody(Context *ctx, QIODevice *body)
{
    Q_D(EngineHttp);
    Q_UNUSED(ctx)

    int *id = static_cast<int*>(ctx->engineData());
    EngineHttpRequest *req = d->requests.value(*id);
    QTcpSocket *socket = req->m_socket;

    body->seek(0);

    char block[4096];
    while (!body->atEnd()) {
        qint64 in = body->read(block, sizeof(block));
        if (in <= 0)
            break;

        if (in != socket->write(block, in)) {
            qCWarning(CUTELYST_ENGINE_HTTP) << "Failed to write body";
            break;
        }
    }

    req->finish();
}

void EngineHttp::removeConnection()
{
    Q_D(EngineHttp);
    EngineHttpRequest *req = static_cast<EngineHttpRequest*>(sender());
    if (req) {
        d->requests.take(req->connectionId());
    }

    if (d->requests.size() <= 5 && d->servers.first()->signalsBlocked()) {
        qCDebug(CUTELYST_ENGINE_HTTP) << "unblock signals" << QCoreApplication::applicationPid();
        d->servers.first()->blockSignals(false);
    }
}

void EngineHttp::processRequest(void *requestData, const QUrl &url, const QByteArray &method, const QByteArray &protocol, const Headers &headers, QIODevice *body)
{
    Engine::processRequest(QString::fromLatin1(method),
                           url.path(),
                           url.query().toLatin1(),
                           QString::fromLatin1(protocol),
                           url.scheme() == QLatin1String("https"),
                           url.host(),
                           QString(),
                           0,
                           QString(),
                           headers,
                           time(),
                           body,
                           requestData);
}

void EngineHttp::onNewServerConnection()
{
    Q_D(EngineHttp);

    QTcpServer *server = static_cast<QTcpServer*>(sender());
    qCDebug(CUTELYST_ENGINE_HTTP) << "onNewServerConnection worker number" << QCoreApplication::applicationPid();
    QTcpSocket *socket = server->nextPendingConnection();
    if (socket) {
        if (d->requests.size() > 5) {
            qCDebug(CUTELYST_ENGINE_HTTP) << "block signals" << QCoreApplication::applicationPid();
            server->blockSignals(true);
        }

        EngineHttpRequest *tcpSocket = new EngineHttpRequest(socket);
        d->requests.insert(socket->socketDescriptor(), tcpSocket);
        connect(tcpSocket, &EngineHttpRequest::requestReady,
                this, &EngineHttp::processRequest);
        connect(tcpSocket, &EngineHttpRequest::destroyed,
                this, &EngineHttp::removeConnection);

//        int workerPos = d->currentChild++ % 4;
//        CutelystChildProcess *worker = d->child.at(workerPos);
//        qDebug() << "worker number" << workerPos << socket->socketDescriptor();

//        if (worker->sendFD(socket->socketDescriptor())) {
//            qDebug() << "fd sent" << d->currentChild;
//        }
//        delete socket;
    }
}

EngineHttpRequest::EngineHttpRequest(QTcpSocket *socket) :
    QObject(socket),
    m_socket(socket),
    m_finishedHeaders(false),
    m_processing(false),
    m_connectionId(socket->socketDescriptor()),
    m_bufLastIndex(0)
{
    connect(socket, &QTcpSocket::readyRead,
            this, &EngineHttpRequest::process);
    connect(socket, &QTcpSocket::bytesWritten,
            this, &EngineHttpRequest::timeout);
    connect(&m_timeoutTimer, &QTimer::timeout,
            this, &EngineHttpRequest::timeout);
    m_timeoutTimer.setInterval(5000);
    m_timeoutTimer.setSingleShot(true);
    m_timeoutTimer.start();
}

int EngineHttpRequest::connectionId()
{
    return m_connectionId;
}

bool EngineHttpRequest::processing()
{
    return m_processing;
}

void EngineHttpRequest::finish()
{
    m_processing = false;
    if (!m_buffer.isNull() && m_socket->bytesAvailable() == 0) {
        QTimer::singleShot(0, this, &EngineHttpRequest::process);
    } else {
        m_timeoutTimer.start();
    }
}

void EngineHttpRequest::process()
{
//    qDebug() << Q_FUNC_INFO << connectionId() << m_processing << m_buffer.size() << sender() << bytesAvailable();
//    qDebug() << Q_FUNC_INFO << m_socket->state() << m_socket->bytesAvailable();
//    m_socket->waitForReadyRead(500);
    m_buffer.append(m_socket->readAll());

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

            QRegularExpression methodProtocolRE(QStringLiteral("(\\w+)\\s+(.*)(?:\\s+(HTTP.*))$"));
            QRegularExpression methodRE(QStringLiteral("(\\w+)\\s+(.*)"));
            bool badRequest = false;
            QRegularExpressionMatch match = methodProtocolRE.match(QString::fromLatin1(section));
            if (match.hasMatch()) {
                m_method = match.captured(1).toLocal8Bit();
                m_path = match.captured(2);
                m_protocol = match.captured(3).toLocal8Bit();
            } else {
                match = methodRE.match(QString::fromLatin1(section));
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
            QString section = QString::fromLatin1(m_buffer.mid(m_bufLastIndex, newLine - m_bufLastIndex - 1));
//            qDebug() << "[header] " << section << section.isEmpty();
            m_bufLastIndex = newLine + 1;

            if (!section.isEmpty()) {
                m_headers[section.section(QLatin1Char(':'), 0, 0)] = section.section(QLatin1Char(':'), 1).trimmed();
            } else {
                m_bodySize = m_headers.contentLength();
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
    if (m_headers.contains(QStringLiteral("Host"))) {
        url = QUrl(QLatin1String("http://") % m_headers.header(QStringLiteral("Host")) % m_path, QUrl::StrictMode);
    } else {
        url = QUrl(QLatin1String("http://") % QHostInfo::localHostName() % m_path, QUrl::StrictMode);
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
                 new QBuffer(&m_body));

    m_body.clear();
    m_headers.clear();
    m_method.clear();
    m_protocol.clear();
}

void EngineHttpRequest::timeout()
{
    QTimer *timer = qobject_cast<QTimer*>(sender());
    if (timer && m_socket->bytesToWrite() == 0 && m_socket->bytesAvailable() == 0) {
        m_socket->close();
        deleteLater();
    } else {
        m_timeoutTimer.start();
    }
}
