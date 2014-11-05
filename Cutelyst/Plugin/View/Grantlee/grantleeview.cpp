/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "grantleeview_p.h"

#include "context.h"
#include "action.h"
#include "response.h"

#include <QString>
#include <QStringBuilder>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_GRANTLEE, "cutelyst.grantlee")

using namespace Cutelyst;

GrantleeView::GrantleeView(QObject *parent) :
    ViewInterface(parent),
    d_ptr(new GrantleeViewPrivate)
{
    Q_D(GrantleeView);

    d->loader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);

    d->engine = new Grantlee::Engine(this);
    d->engine->addTemplateLoader(d->loader);
}

GrantleeView::~GrantleeView()
{
    delete d_ptr;
}

QString GrantleeView::includePath() const
{
    Q_D(const GrantleeView);
    return d->includePath;
}

void GrantleeView::setIncludePath(const QString &path)
{
    Q_D(GrantleeView);
    d->loader->setTemplateDirs(QStringList() << path);
    d->includePath = path;
}

QString GrantleeView::templateExtension() const
{
    Q_D(const GrantleeView);
    return d->extension;
}

void GrantleeView::setTemplateExtension(const QString &extension)
{
    Q_D(GrantleeView);
    d->extension = extension;
}

QString GrantleeView::wrapper() const
{
    Q_D(const GrantleeView);
    return d->wrapper;
}

void GrantleeView::setWrapper(const QString &name)
{
    Q_D(GrantleeView);
    d->wrapper = name;
}

void GrantleeView::setCache(bool enable)
{
    Q_D(GrantleeView);

    if (enable != d->cache.isNull()) {
        return; // already enabled
    }

    delete d->engine;
    d->engine = new Grantlee::Engine(this);

    if (enable) {
        d->cache = QSharedPointer<Grantlee::CachingLoaderDecorator>(new Grantlee::CachingLoaderDecorator(d->loader));
        d->engine->addTemplateLoader(d->cache);
    } else {
        d->cache.clear();
        d->engine->addTemplateLoader(d->loader);
    }
}

bool GrantleeView::isCaching() const
{
    Q_D(const GrantleeView);
    return !d->cache.isNull();
}

bool GrantleeView::render(Context *ctx)
{
    Q_D(GrantleeView);

    QVariantHash &stash = ctx->stash();
    QString templateFile = stash.value(QStringLiteral("template")).toString();
    if (templateFile.isEmpty()) {
        if (ctx->action() && !ctx->action()->reverse().isEmpty()) {
            templateFile = ctx->action()->reverse() % d->extension;
        }

        if (templateFile.isEmpty()) {
            qCCritical(CUTELYST_GRANTLEE) << "Cannot render template, template name or template stash key not defined";
            return false;
        }
    }

    stash.insert(QStringLiteral("ctx"), QVariant::fromValue(ctx));
    Grantlee::Context gCtx(stash);

    Grantlee::Template tmpl;
    if (d->wrapper.isEmpty()) {
        tmpl = d->engine->loadByName(templateFile);
    } else {
        tmpl = d->engine->loadByName(d->wrapper);

        gCtx.insert(QStringLiteral("template"), templateFile);
    }

    ctx->res()->body() = tmpl->render(&gCtx).toUtf8();

    if (tmpl->error() != Grantlee::NoError) {
        qCCritical(CUTELYST_GRANTLEE) << "Error while rendering template" << tmpl->errorString();
    }

    return tmpl->error() == Grantlee::NoError;
}
