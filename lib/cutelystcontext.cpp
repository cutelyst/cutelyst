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

#include "cutelystcontext_p.h"

#include "cutelystrequest.h"
#include "cutelystaction.h"
#include "cutelystdispatcher.h"

#include <QStringList>

CutelystContext::CutelystContext(QObject *parent) :
    QObject(parent),
    d_ptr(new CutelystContextPrivate(this))
{
}

CutelystContext::~CutelystContext()
{
    delete d_ptr;
}

bool CutelystContext::error() const
{
    return false;
}

bool CutelystContext::state() const
{
    return true;
}

QStringList CutelystContext::args() const
{
    Q_D(const CutelystContext);
    return d->request->args();
}

CutelystRequest *CutelystContext::request() const
{
    Q_D(const CutelystContext);
    return d->request;
}

CutelystRequest *CutelystContext::req() const
{
    Q_D(const CutelystContext);
    return d->request;
}

CutelystResponse *CutelystContext::response() const
{
    Q_D(const CutelystContext);
    return d->response;
}

CutelystAction *CutelystContext::action() const
{
    Q_D(const CutelystContext);
    return d->action;
}

CutelystDispatcher *CutelystContext::dispatcher() const
{
    Q_D(const CutelystContext);
    return d->dispatcher;
}

QString CutelystContext::ns() const
{
    Q_D(const CutelystContext);
    return d->action->ns();
}

QString CutelystContext::match() const
{
    Q_D(const CutelystContext);
    return d->match;
}

void CutelystContext::dispatch()
{
    Q_D(CutelystContext);
    d->dispatcher->dispatch(this);
}

bool CutelystContext::forward(const QString &action, const QStringList &arguments)
{
    Q_D(CutelystContext);
    return d->dispatcher->forward(this, action, arguments);
}

CutelystAction *CutelystContext::getAction(const QString &action, const QString &ns)
{
    Q_D(CutelystContext);
    return d->dispatcher->getAction(action, ns);
}

QList<CutelystAction *> CutelystContext::getActions(const QString &action, const QString &ns)
{
    Q_D(CutelystContext);
    return d->dispatcher->getActions(action, ns);
}

CutelystContextPrivate::CutelystContextPrivate(CutelystContext *parent) :
    action(0)
{
}
