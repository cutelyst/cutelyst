#include "cutelystrequest.h"

#include <QHostAddress>
#include <QTimer>

// TODO make this configurable
#define TIMEOUT 3000

CutelystRequest::CutelystRequest(QObject *parent) :
    QObject(parent)
{
}

CutelystRequest::~CutelystRequest()
{
}

QHostAddress CutelystRequest::peerAddress() const
{
    return m_socket->peerAddress();
}

QString CutelystRequest::peerName() const
{
    return m_socket->peerName();
}

quint16 CutelystRequest::peerPort() const
{
    return m_socket->peerPort();
}

QVariant CutelystRequest::cookie(const QString &name) const
{
    return cookies().value(name);
}

QVariantHash CutelystRequest::cookies() const
{

}

QVariant CutelystRequest::header(const QString &name) const
{
    return headers().value(name);
}

QVariantHash CutelystRequest::headers() const
{

}
