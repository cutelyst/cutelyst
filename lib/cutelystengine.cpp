#include "cutelystengine_p.h"

#include "cutelystrequest.h"

CutelystEngine::CutelystEngine(int socket, QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystEnginePrivate(this))
{
    Q_D(CutelystEngine);

    d->socket = new QTcpSocket(this);
    d->valid = d->socket->setSocketDescriptor(socket);
    if (d->valid) {
        connect(d->socket, &QTcpSocket::readyRead,
                this, &CutelystEngine::readyRead);
    }
}

CutelystEngine::~CutelystEngine()
{
    Q_D(CutelystEngine);

    d->socket->waitForBytesWritten();
    d->socket->close();
    delete d_ptr;
}

bool CutelystEngine::isValid() const
{
    Q_D(const CutelystEngine);
    return d->valid;
}

CutelystRequest *CutelystEngine::request() const
{
    Q_D(const CutelystEngine);
    return d->request;
}

qint64 CutelystEngine::write(const QByteArray &data)
{
    Q_D(CutelystEngine);

    return d->socket->write(data);
}

void CutelystEngine::readyRead()
{
    Q_D(CutelystEngine);
    parse(d->socket->readAll());
}

CutelystEnginePrivate::CutelystEnginePrivate(CutelystEngine *parent) :
    q_ptr(parent),
    request(new CutelystRequest)
{

}

CutelystEnginePrivate::~CutelystEnginePrivate()
{
}

quint16 CutelystEngine::peerPort() const
{
    Q_D(const CutelystEngine);
    return d->socket->peerPort();
}

QString CutelystEngine::peerName() const
{
    Q_D(const CutelystEngine);
    return d->socket->peerName();
}

QHostAddress CutelystEngine::peerAddress() const
{
    Q_D(const CutelystEngine);
    return d->socket->peerAddress();
}
