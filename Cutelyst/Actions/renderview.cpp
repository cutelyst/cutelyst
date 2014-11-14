/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "renderview_p.h"

#include "response.h"
#include "view.h"
#include "application.h"

#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_RENDERVIEW, "cutelyst.renderview")

using namespace Cutelyst;

RenderView::RenderView() :
    d_ptr(new RenderViewPrivate)
{
}

RenderView::~RenderView()
{
    delete d_ptr;
}

bool RenderView::init(const QVariantHash &args)
{
    Q_D(RenderView);

    QMap<QByteArray, QByteArray> attributes;
    attributes = args.value("attributes").value<QMap<QByteArray, QByteArray> >();
    d->view = attributes.value("View");
}

bool RenderView::doExecute(Cutelyst::Context *ctx) const
{
    Q_D(const RenderView);

    if (!Action::doExecute(ctx)) {
        return false;
    }

    Response *res = ctx->res();
    if (res->contentType().isEmpty()) {
        res->setContentType("text/html; charset=utf-8");
    }

    if (ctx->req()->method() == "HEAD") {
        return true;
    }

    if (res->hasBody()) {
        return true;
    }

    quint16 status = res->status();
    if (status == 204 || (status >= 300 && status < 400)) {
        return true;
    }

    View *view = ctx->view();
    if (view) {
        return view->render(ctx);
    }

    view = ctx->app()->view(d->view);
    if (view) {
        return view->render(ctx);
    }

    qCCritical(CUTELYST_RENDERVIEW) << "Could not find a view to render.";
    res->setStatus(500);
    return false;
}
