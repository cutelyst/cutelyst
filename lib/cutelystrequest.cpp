#include "cutelystrequest_p.h"
#include "cutelystengine.h"

// TODO make this configurable
#define TIMEOUT 3000

CutelystRequest::CutelystRequest() :
    d_ptr(new CutelystRequestPrivate)
{
}

CutelystRequest::CutelystRequest(CutelystRequestPrivate *prv) :
    d_ptr(prv)
{

}

CutelystRequest::~CutelystRequest()
{
}

QHostAddress CutelystRequest::peerAddress() const
{
    Q_D(const CutelystRequest);
    return d->engine->peerAddress();
}

QString CutelystRequest::peerName() const
{
    Q_D(const CutelystRequest);
    return d->engine->peerName();
}

quint16 CutelystRequest::peerPort() const
{
    Q_D(const CutelystRequest);
    return d->engine->peerPort();
}

QString CutelystRequest::cookie(const QString &key) const
{
    Q_D(const CutelystRequest);
    return d->cookies.value(key);
}

QHash<QString, QString> CutelystRequest::cookies() const
{
    Q_D(const CutelystRequest);
    return d->cookies;
}

QString CutelystRequest::header(const QString &key) const
{
    Q_D(const CutelystRequest);
    return d->headers.value(key);
}

QHash<QString, QString> CutelystRequest::headers() const
{
    Q_D(const CutelystRequest);
    return d->headers;
}
