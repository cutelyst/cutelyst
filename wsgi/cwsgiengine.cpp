#include "cwsgiengine.h"

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

using namespace CWSGI;

CWsgiEngine::CWsgiEngine(const QVariantMap &opts, QObject *parent) : Engine(opts)
{
    m_proto = new ProtocolHttp(this);
    m_app = qobject_cast<Application*>(parent);
}

int CWsgiEngine::workerId() const
{
    return m_workerId;
}

int CWsgiEngine::workerCore() const
{
    return m_workerCore;
}

void CWsgiEngine::setTcpSockets(const QVector<QTcpServer *> sockets)
{
    m_sockets = sockets;
}

void CWsgiEngine::forked()
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
    }
}

bool CWsgiEngine::finalizeHeaders(Context *ctx)
{
    //    qDebug() << Q_FUNC_INFO;
    //    qDebug() << ctx->request()->engineData();
    //    qDebug() << static_cast<QObject*>(ctx->request()->engineData());

    auto conn = static_cast<QIODevice*>(ctx->request()->engineData());
    Response *res = ctx->res();

//     status = QByteArrayLiteral("HTTP/1.1 ");
//    QByteArray code = statusCode(res->status());
//    status.reserve(code.size() + 9);
//    status.append(code);
    conn->write("HTTP/1.1 ", 9);
//    qDebug() << ret << 9;

    conn->write(statusCode(res->status()));
//        return false;
//    }

    if (!Engine::finalizeHeaders(ctx)) {
        return false;
    }

    auto sock = qobject_cast<TcpSocket*>(conn);
    const auto headers = res->headers().map();
    if (sock->headerClose == 1) {
        sock->headerClose = 0;
    }

    auto it = headers.constBegin();
    auto endIt = headers.constEnd();
    while (it != endIt) {
        const QString key = it.key();
        const QString value = it.value();
        if (sock->headerClose == 0 && key == QLatin1String("connection")) {
            if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
                sock->headerClose = 2;
            } else {
                sock->headerClose = 1;
            }
        }
        QString ret(QLatin1String("\r\n") + camelCaseHeader(key) + QLatin1String(": ") + value);
        conn->write(ret.toLatin1());

        ++it;
    }

//    if (QString::compare(sock->headers.connection(), QLatin1String("close"), Qt::CaseInsensitive) == 0 ||
//            QString::compare(headers.value(QStringLiteral("connection")), QLatin1String("close"), Qt::CaseInsensitive) == 0) {
//        sock->headerClose = true;
//    }

    conn->write("\r\n\r\n", 4);

    return true;
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
    return true;
}
