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

#include "cutelystrequest.h"
#include "cutelystdispatcher.h"
#include "cutelystcontext.h"

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

void CutelystEngine::dispatch(CutelystContext *c)
{
    Q_D(CutelystEngine);
    d->dispatcher->prepareAction(c);
    c->dispatch();
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
