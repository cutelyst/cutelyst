/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include "viewemailtemplate_p.h"

#include <Cutelyst/Context>

#include <SimpleMail/mimemessage.h>
#include <SimpleMail/emailaddress.h>
#include <SimpleMail/mimetext.h>

#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_VIEW_EMAILTEMPLATE, "cutelyst.view.emailtemplate")

using namespace Cutelyst;

ViewEmailTemplate::ViewEmailTemplate(QObject *parent, const QString &name) : ViewEmail(new ViewEmailTemplatePrivate, parent, name)
{

}

QString ViewEmailTemplate::templatePrefix() const
{
    Q_D(const ViewEmailTemplate);
    return d->templatePrefix;
}

void ViewEmailTemplate::setTemplatePrefix(const QString &prefix)
{
    Q_D(ViewEmailTemplate);
    d->templatePrefix = prefix;
}

QString ViewEmailTemplate::defaultView() const
{
    Q_D(const ViewEmailTemplate);
    return d->defaultView;
}

void ViewEmailTemplate::setDefaultView(const QString &view)
{
    Q_D(ViewEmailTemplate);
    d->defaultView = view;
}

MimePart *generatePart(Context *c, const ViewEmailTemplatePrivate *d, const QVariantHash &partHash)
{
    const QString defaultView = d->defaultView;
    const QString defaultContentType = d->defaultContentType;
    const QString defaultCharset = d->defaultCharset;

    View *view = nullptr;
    auto viewIt = partHash.constFind(QStringLiteral("view"));
    if (viewIt != partHash.constEnd() && !viewIt.value().toString().isEmpty()) {
        // use the view specified for the email part
        const QString viewString = viewIt.value().toString();
        qCDebug(CUTELYST_VIEW_EMAILTEMPLATE) << "Using specified view" << viewString << "for rendering.";
        view = c->view(viewString);
    } else if (!defaultView.isEmpty()) {
        // if none specified use the configured default view
        qCDebug(CUTELYST_VIEW_EMAILTEMPLATE) << "Using default view" << defaultView << "for rendering.";
        view = c->view(defaultView);
    } else {
        // else fallback to Cutelysts default view
        qCDebug(CUTELYST_VIEW_EMAILTEMPLATE) << "Using Cutelysts default view" << defaultView << "for rendering.";
        view = c->view();
    }

    // validate the per template view
    if (!view) {
        c->error(QLatin1String("Could not find a view to render"));
        return nullptr;
    }

    QString templateString = partHash.value(QStringLiteral("template")).toString();;
    // prefix with template_prefix if configured
    if (!d->templatePrefix.isEmpty()) {
        templateString = d->templatePrefix % QLatin1Char('/') % templateString;
    }

    // render the email part
    const QVariantHash currentStash = c->stash();
    c->stash(partHash);
    c->setStash(QStringLiteral("template"), templateString);
    QByteArray output = view->render(c);
    c->stash() == currentStash;


    MimePart *part = new MimePart();
    part->setContent(output);

    return part;
}

QByteArray ViewEmailTemplate::render(Context *c) const
{
    Q_D(const ViewEmailTemplate);

    const QVariantHash email = c->stash(d->stashKey).toHash();
    const QVariantHash templateHash = email.value(QStringLiteral("template")).toHash();
    const QVariantList templateList = email.value(QStringLiteral("templates")).toList();
    if (templateHash.isEmpty() || templateList.isEmpty()) {
        return ViewEmail::render(c);
    }

    QVariantList parts;
    if (!templateList.isEmpty() && templateList.first().type() == QVariant::Hash) {
        // multipart API

        Q_FOREACH (const QVariant &part, templateList) {
            const QVariantHash partHash = part.toHash();
            MimePart *partObj = generatePart(c, d, partHash);
            parts.append(QVariant::fromValue(partObj));
        }

    } else if (!templateHash.isEmpty()) {
        // single part API
        MimePart *partObj = generatePart(c, d, templateHash);
        parts.append(QVariant::fromValue(partObj));
    }
    c->setStash(QStringLiteral("parts"), parts);

    return ViewEmail::render(c);
}
