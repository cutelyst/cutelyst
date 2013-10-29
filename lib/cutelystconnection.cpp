#include "cutelystconnection_p.h"

#include "cutelystrequest.h"

CutelystConnection::CutelystConnection(int socket, QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystConnectionPrivate(this))
{
    Q_D(CutelystConnection);

    d->socket = new QTcpSocket(this);
    d->valid = d->socket->setSocketDescriptor(socket);
    if (d->valid) {
        connect(d->socket, &QTcpSocket::readyRead,
                this, &CutelystConnection::readyRead);
    }
}

CutelystConnection::~CutelystConnection()
{
    Q_D(CutelystConnection);

    d->socket->waitForBytesWritten();
    d->socket->close();
    delete d_ptr;
}

bool CutelystConnection::isValid() const
{
    Q_D(const CutelystConnection);
    return d->valid;
}

CutelystRequest *CutelystConnection::request() const
{
    Q_D(const CutelystConnection);
    return d->request;
}

qint64 CutelystConnection::write(const QByteArray &data)
{
    Q_D(CutelystConnection);

    return d->socket->write(data);
}

void CutelystConnection::readyRead()
{
    Q_D(CutelystConnection);
    parse(d->socket->readAll());
}

CutelystConnectionPrivate::CutelystConnectionPrivate(CutelystConnection *parent) :
    q_ptr(parent),
    request(new CutelystRequest)
{

}

CutelystConnectionPrivate::~CutelystConnectionPrivate()
{
}
