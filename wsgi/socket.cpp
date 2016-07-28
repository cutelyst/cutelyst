#include "socket.h"

TcpSocket::TcpSocket(QObject *parent) : QTcpSocket(parent)
{

}

Socket::Socket()
{
    buf = new char[4096];
}

Socket::~Socket()
{
    delete [] buf;
}
