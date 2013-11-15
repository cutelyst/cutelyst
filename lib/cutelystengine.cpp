/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "cutelystengine_p.h"

#include "cutelystrequest_p.h"
#include "cutelystresponse.h"
#include "cutelystdispatcher.h"
#include "cutelystcontext_p.h"

#include <QUrl>

CutelystEngine::CutelystEngine(int socket, CutelystDispatcher *dispatcher, QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystEnginePrivate(this))
{
    Q_D(CutelystEngine);

    d->dispatcher = dispatcher;
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

void CutelystEngine::handleRequest(CutelystRequest *request)
{
    Q_D(CutelystEngine);
    CutelystContext *c = new CutelystContext(this, d->dispatcher);

    c->handleRequest(request, new CutelystResponse);
}

CutelystRequest *CutelystEngine::createRequest(const QUrl &url, const QString &method, const QString &protocol, const QHash<QString, QByteArray> &headers, const QByteArray &body) const
{
    // Parse the query (GET) parameters ie "?foo=bar&bar=baz"
    QMultiHash<QString, QString> queryParam;
    foreach (const QString &parameter, url.query(QUrl::FullyEncoded).split(QLatin1Char('&'))) {
        if (parameter.isEmpty()) {
            continue;
        }

        QStringList parts = parameter.split(QLatin1Char('='));
        if (parts.size() == 2) {
            queryParam.insertMulti(QUrl::fromPercentEncoding(parts.at(0).toUtf8()),
                                   QUrl::fromPercentEncoding(parts.at(1).toUtf8()));
        } else {
            queryParam.insertMulti(QUrl::fromPercentEncoding(parts.first().toUtf8()),
                                   QString());
        }
    }

    QMultiHash<QString, QString> bodyParam;
    if (headers.value(QLatin1String("Content-Type")) == "application/x-www-form-urlencoded") {
        // Parse the query (BODY) "application/x-www-form-urlencoded"
        // parameters ie "?foo=bar&bar=baz"
        foreach (const QByteArray &parameter, body.split('&')) {
            if (parameter.isEmpty()) {
                continue;
            }

            QList<QByteArray> parts = parameter.split('=');
            if (parts.size() == 2) {
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.at(0)),
                                      QUrl::fromPercentEncoding(parts.at(1)));
            } else {
                bodyParam.insertMulti(QUrl::fromPercentEncoding(parts.first()),
                                      QString());
            }
        }
    }

    QByteArray cookies = headers.value(QLatin1String("Cookie"));

    CutelystRequestPrivate *requestPriv = new CutelystRequestPrivate;
    requestPriv->engine = this;
    requestPriv->method = method;
    requestPriv->url = url;
    requestPriv->protocol = protocol;
    requestPriv->queryParam = queryParam;
    requestPriv->bodyParam = bodyParam;
    requestPriv->param = queryParam + bodyParam;
    requestPriv->headers = headers;
    requestPriv->cookies = QNetworkCookie::parseCookies(cookies.replace(';', '\n'));
    requestPriv->body = body;

    return new CutelystRequest(requestPriv);
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

void CutelystEngine::finalizeCookies(CutelystContext *c)
{
    foreach (const QNetworkCookie &cookie, c->response()->cookies()) {
        c->response()->addHeaderValue(QLatin1String("Set-Cookie"), cookie.toRawForm());
    }
}
