#include "tcpserver.h"
#include "socket.h"
#include "protocolhttp.h"

#include <Cutelyst/Engine>
#include <QDateTime>

TcpServer::TcpServer(QObject *parent) : QTcpServer(parent)
{
    m_proto = new ProtocolHttp(this);
    m_engine = qobject_cast<CuteEngine*>(parent);
}

void TcpServer::incomingConnection(qintptr handle)
{
//    qDebug() << Q_FUNC_INFO << handle << thread();
    auto sock = new TcpSocket(this);
    sock->setSocketDescriptor(handle);
    sock->start = QDateTime::currentMSecsSinceEpoch();

//    auto server = qobject_cast<QTcpServer*>(sender());
//    QTcpSocket *conn = server->nextPendingConnection();
//    if (conn) {
        connect(sock, &QTcpSocket::disconnected, sock, &QTcpSocket::deleteLater);
//        TcpSocket *sock = qobject_cast<TcpSocket*>(conn);
        sock->engine = m_engine;
        static QString serverAddr = serverAddress().toString();
        sock->serverAddress = serverAddr;
        connect(sock, &QIODevice::readyRead, m_proto, &Protocol::readyRead);
//    }
//    addPendingConnection(sock);
}
