#include "tcpserver.h"
#include "socket.h"
#include "protocolhttp.h"

#include <Cutelyst/Engine>
#include <QDateTime>
#include <QSocketNotifier>

using namespace CWSGI;

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    m_proto = new ProtocolHttp(this);
    m_engine = qobject_cast<CWsgiEngine*>(parent);
}

void TcpServer::incomingConnection(qintptr handle)
{
//    qDebug() << Q_FUNC_INFO << handle << thread();
    TcpSocket *sock;
    if (m_socks.size()) {
        sock = m_socks.takeLast();
    } else {
        sock = new TcpSocket(this);
        sock->engine = m_engine;
        static QString serverAddr = serverAddress().toString();
        sock->serverAddress = serverAddr;
        connect(sock, &QIODevice::readyRead, m_proto, &Protocol::readyRead);
        connect(sock, &TcpSocket::finished, this, &TcpServer::enqueue);
    }

//    auto requestNotifier = new QSocketNotifier(handle, QSocketNotifier::Read, sock);
//    connect(requestNotifier, &QSocketNotifier::activated, sock, &QIODevice::readyRead);
//sock->fd = handle;
//    connect(requestNotifier, &QSocketNotifier::activated,
//            [=]() {
//        sock->readyRead();
//    });

//    qDebug() << Q_FUNC_INFO << handle << sock;

    sock->setSocketDescriptor(handle);
    sock->start = QDateTime::currentMSecsSinceEpoch();

//    auto server = qobject_cast<QTcpServer*>(sender());
//    QTcpSocket *conn = server->nextPendingConnection();
//    if (conn) {
//    connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
//    connect(sock, &QTcpSocket::disconnected, this, &TcpServer::enqueue);
//        TcpSocket *sock = qobject_cast<TcpSocket*>(conn);
//        sock->engine = m_engine;
//        static QString serverAddr = serverAddress().toString();
//        sock->serverAddress = serverAddr;
//        connect(sock, &QIODevice::readyRead, m_proto, &Protocol::readyRead);
//    }
        //    addPendingConnection(sock);
}

void TcpServer::enqueue()
{
    m_socks.push_back(qobject_cast<TcpSocket*>(sender()));
}
