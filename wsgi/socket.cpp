#include "socket.h"

#include <QDebug>

using namespace CWSGI;

TcpSocket::TcpSocket(QObject *parent) : QTcpSocket(parent)
{
    connect(this, &QTcpSocket::disconnected, this, &TcpSocket::socketDisconnected);
}

Socket::Socket()
{
    buf = new char[4096];
}

Socket::~Socket()
{
    delete [] buf;
}

void TcpSocket::socketDisconnected()
{
    qDebug() << Q_FUNC_INFO << processing;
    if (!processing) {
        Q_EMIT finished();
    }
}
