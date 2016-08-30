#include "cwsgiengine.h"

#include "protocolhttp.h"
#include "tcpserver.h"

#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Request>
#include <Cutelyst/Application>

#include <QCoreApplication>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLocalServer>
#include <QLocalSocket>

using namespace CWSGI;

CWsgiEngine::CWsgiEngine(Application *app, int workerCore, const QVariantMap &opts) : Engine(app, workerCore, opts)
{
    m_proto = new ProtocolHttp(this);
}

int CWsgiEngine::workerId() const
{
    return m_workerId;
}

void CWsgiEngine::setTcpSockets(const QVector<QTcpServer *> sockets)
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
        auto server = new TcpServer(this);
        server->setSocketDescriptor(socket->socketDescriptor());
    }

    Q_EMIT listening();
}

void CWsgiEngine::postFork()
{
    if (!postForkApplication()) {
        // CHEAP
        QCoreApplication::exit(15);
    }
}

bool CWsgiEngine::finalizeHeadersWrite(Context *c, quint16 status, const Headers &headers, void *engineData)
{
    auto conn = static_cast<QIODevice*>(engineData);

    conn->write("HTTP/1.1 ", 9);
    int msgLen;
    const char *msg = httpStatusMessage(status, &msgLen);
    conn->write(msg, msgLen);

    auto sock = qobject_cast<TcpSocket*>(conn);
    const auto headersData = headers.data();
    if (sock->headerClose == 1) {
        sock->headerClose = 0;
    }

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
        }
        QString ret(QLatin1String("\r\n") + camelCaseHeader(key) + QLatin1String(": ") + value);
        conn->write(ret.toLatin1());

        ++it;
    }

    return conn->write("\r\n\r\n", 4) == 4;
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
