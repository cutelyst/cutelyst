#include "cuteengine.h"

#include "protocolhttp.h"
#include "tcpserver.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Request>
#include <Cutelyst/Application>

#include <QTcpServer>
#include <QTcpSocket>
#include <QLocalServer>
#include <QLocalSocket>

CuteEngine::CuteEngine(const QVariantMap &opts, QObject *parent) : Engine(opts)
{
    m_proto = new ProtocolHttp(this);
    m_app = qobject_cast<Application*>(parent);
}

int CuteEngine::workerId() const
{
    return m_workerId;
}

int CuteEngine::workerCore() const
{
    return m_workerCore;
}

void CuteEngine::setTcpSockets(const QVector<QTcpServer *> sockets)
{
    m_sockets = sockets;
}

void CuteEngine::forked()
{
    if (workerCore() > 0) {
        m_app = qobject_cast<Application *>(m_app->metaObject()->newInstance());
        if (!m_app) {
            qFatal("*** FATAL *** Could not create a NEW instance of your Cutelyst::Application, "
                   "make sure your constructor has Q_INVOKABLE macro or disable threaded mode.\n");
            return;
        }

        // Move the application and it's children to this thread
        m_app->moveToThread(thread());

        // We can now set a parent
        m_app->setParent(this);

        // init and postfork
        if (!initApplication(m_app, true)) {
            qCritical() << "Failed to init application on a different thread than main. Are you sure threaded mode is supported in this application?";
            return;
        }
    } else {
        postForkApplication();
    }

    for (QTcpServer *socket : m_sockets) {
        auto server = new TcpServer(this);
        server->setSocketDescriptor(socket->socketDescriptor());
        connect(server, &TcpServer::newConnection, this, &CuteEngine::newconnectionTcp);
    }
}

bool CuteEngine::finalizeHeaders(Context *ctx)
{
    //    qDebug() << Q_FUNC_INFO;
    //    qDebug() << ctx->request()->engineData();
    //    qDebug() << static_cast<QObject*>(ctx->request()->engineData());

    QIODevice *conn = static_cast<QIODevice*>(ctx->request()->engineData());
    Response *res = ctx->res();

    QByteArray status = QByteArrayLiteral("HTTP/1.1 ");
    QByteArray code = statusCode(res->status());
    status.reserve(code.size() + 9);
    status.append(code);
    if (conn->write(status) != status.size()) {
        return false;
    }

    if (!Engine::finalizeHeaders(ctx)) {
        return false;
    }

    const auto headers = res->headers().map();
    auto it = headers.constBegin();
    auto endIt = headers.constEnd();
    while (it != endIt) {
        QByteArray key = it.key().toLatin1();
        camelCaseByteArrayHeader(key);
        QByteArray value = it.value().toLatin1();
        QByteArray buf;
        buf.reserve(key.size() + 2 + value.size() + 2);
        buf.append("\r\n", 2);
        buf.append(key);
        buf.append(": ", 2);
        buf.append(value);

//        conn->write("\r\n", 2);
//        conn->write(key);
//        conn->write(": ", 2);
//        conn->write(value);

        conn->write(buf);

        ++it;
    }

    conn->write("\r\n\r\n", 4);

    return true;
}

qint64 CuteEngine::doWrite(Context *c, const char *data, qint64 len, void *engineData)
{
    QIODevice *conn = static_cast<QIODevice*>(engineData);
    //    qDebug() << Q_FUNC_INFO << QByteArray(data,len);
    conn->write(data, len);
    //    conn->waitForBytesWritten(200);
}

bool CuteEngine::init()
{
    return true;
}

void CuteEngine::newconnectionTcp()
{
    auto server = qobject_cast<QTcpServer*>(sender());
    QTcpSocket *conn = server->nextPendingConnection();
    if (conn) {
        connect(conn, &QTcpSocket::disconnected, conn, &QTcpSocket::deleteLater);
        TcpSocket *sock = qobject_cast<TcpSocket*>(conn);
        sock->engine = this;
        static QString serverAddress = server->serverAddress().toString();
        sock->serverAddress = serverAddress;
        connect(conn, &QIODevice::readyRead, m_proto, &Protocol::readyRead);
    }
}

void CuteEngine::newconnectionLocalSocket()
{
//    auto server = qobject_cast<QLocalServer*>(sender());
//    QLocalSocket *conn = server->nextPendingConnection();
//    if (conn) {

//    }
}
