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

bool RenderView::init(Cutelyst::Application *application, const QVariantHash &args)
{
    Q_D(RenderView);

    QMap<QString, QString> attributes;
    attributes = args.value("attributes").value<QMap<QString, QString> >();
    d->view = application->view(attributes.value("View"));

    return Action::init(application, args);
}

bool RenderView::doExecute(Cutelyst::Context *ctx)
{
    Q_D(const RenderView);

    if (!Action::doExecute(ctx)) {
        return false;
    }

    Response *res = ctx->res();
    if (res->contentType().isEmpty()) {
        res->setContentType(QStringLiteral("text/html; charset=utf-8"));
    }

    if (ctx->req()->method() == QStringLiteral("HEAD")) {
        return true;
    }

    if (res->hasBody()) {
        return true;
    }

    quint16 status = res->status();
    if (status == 204 || (status >= 300 && status < 400)) {
        return true;
    }

    // First use the action View attribute
    if (d->view) {
        return d->view->render(ctx);
    }

    // If the above is not set try the
    // view choosen by the user or the
    // application default
    View *view = ctx->view();
    if (view) {
        return view->render(ctx);
    }

    qCCritical(CUTELYST_RENDERVIEW) << "Could not find a view to render.";
    res->setStatus(500);
    return false;
}
