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

#include "cutelystengine.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"
#include "cutelystaction.h"
#include "cutelystdispatcher.h"

#include <QUrl>
#include <QStringBuilder>
#include <QStringList>

CutelystContext::CutelystContext(CutelystEngine *engine, CutelystDispatcher *dispatcher) :
    QObject(engine),
    d_ptr(new CutelystContextPrivate(this))
{
    Q_D(CutelystContext);
    d->engine = engine;
    d->dispatcher = dispatcher;
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

CutelystEngine *CutelystContext::engine() const
{
    Q_D(const CutelystContext);
    return d->engine;
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

void CutelystContext::handleRequest(CutelystRequest *req, CutelystResponse *resp)
{
    Q_D(CutelystContext);

    d->request = req;
    d->response = resp;

    d->dispatcher->prepareAction(this);
    dispatch();
    d->status = finalize();
}

void CutelystContext::finalizeHeaders()
{
    Q_D(CutelystContext);

    CutelystResponse *response = d->response;
    if (response->finalizedHeaders()) {
        return;
    }

    if (!response->redirect().isEmpty()) {
        response->setHeaderValue(QLatin1String("Location"), response->redirect());

        if (!response->hasBody()) {
            QByteArray data;
            data = "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\">"
                   "  <head>"
                   "    <title>Moved</title>"
                   "  </head>"
                   "  <body>"
                   "     <p>This item has moved <a href=";
            data.append(QUrl::toPercentEncoding(response->redirect()));
            data.append(">here</a>.</p>"
                   "  </body>"
                   "</html>");
            response->setContentType(QLatin1String("text/html; charset=utf-8"));
        }
    }

    if (response->hasBody()) {
        response->setContentLength(response->body().size());
    }

    finalizeCookies();

    d->engine->finalizeHeaders(this);
}

void CutelystContext::finalizeCookies()
{
    Q_D(CutelystContext);
    d->engine->finalizeCookies(this);
}

void CutelystContext::finalizeBody()
{
    Q_D(CutelystContext);
    d->engine->finalizeBody(this);
}

void CutelystContext::finalizeError()
{
    Q_D(CutelystContext);
    d->engine->finalizeError(this);
}

int CutelystContext::finalize()
{
    Q_D(CutelystContext);

    finalizeHeaders();

    if (d->request->method() == QLatin1String("HEAD")) {
        d->response->setBody(QByteArray());
    }

    finalizeBody();

    return d->response->status();
}

CutelystContextPrivate::CutelystContextPrivate(CutelystContext *parent) :
    action(0)
{
}
