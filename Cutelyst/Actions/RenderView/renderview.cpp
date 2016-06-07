/*
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include "context.h"
#include "componentfactory.h"

#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_RENDERVIEW, "cutelyst.renderview")

using namespace Cutelyst;

RenderView::RenderView(QObject *parent) : Action(parent)
    , d_ptr(new RenderViewPrivate)
{
    setObjectName(QString::fromLatin1(metaObject()->className()) + QLatin1String("->execute"));
}

RenderView::~RenderView()
{
    delete d_ptr;
}

bool RenderView::init(Cutelyst::Application *application, const QVariantHash &args)
{
    Q_D(RenderView);

    const auto attributes = args.value(QLatin1String("attributes")).value<QMap<QString, QString> >();
    d->view = application->view(attributes.value(QLatin1String("View")));

    return Action::init(application, args);
}

bool RenderView::doExecute(Cutelyst::Context *c)
{
    Q_D(const RenderView);

    if (!Action::doExecute(c)) {
        return false;
    }

    Response *res = c->res();
    if (res->contentType().isNull()) {
        res->setContentType(QStringLiteral("text/html; charset=utf-8"));
    }

    if (c->req()->method() == QLatin1String("HEAD")) {
        return true;
    }

    if (res->hasBody()) {
        return true;
    }

    quint16 status = res->status();
    if (status == 204 || (status >= 300 && status < 400)) {
        return true;
    }

    View *view = c->view();
    if (view) {
        // Fist check if the user set a view
        return c->forward(view);
    } else if (d->view) {
        // Then try to use the action View attribute
        return c->forward(d->view);
    }

    qCCritical(CUTELYST_RENDERVIEW) << "Could not find a view to render.";
    res->setStatus(500);
    return false;
}

#include "moc_renderview.cpp"
