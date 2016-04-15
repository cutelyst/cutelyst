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
    Q_D(ViewEmailTemplate);

    d->defaultContentType = "text/html";
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
        c->error(QStringLiteral("Could not find a view to render"));
        return nullptr;
    }

    QString templateString = partHash.value(QStringLiteral("template")).toString();;
    // prefix with template_prefix if configured
    if (!d->templatePrefix.isEmpty()) {
        templateString = d->templatePrefix + QLatin1Char('/') + templateString;
    }

    // render the email part
    const QVariantHash currentStash = c->stash();
    c->stash(partHash);
    c->setStash(QStringLiteral("template"), templateString);
    QByteArray output = view->render(c);
    if (c->error()) {
        qCDebug(CUTELYST_VIEW_EMAILTEMPLATE) << "Errors" << c->errors();
    }
    c->stash() = currentStash;

    MimePart *part = new MimePart();
    part->setContent(output);

    d->setupAttributes(part, partHash);

    return part;
}

QByteArray ViewEmailTemplate::render(Context *c) const
{
    Q_D(const ViewEmailTemplate);

    QVariantHash email = c->stash(d->stashKey).toHash();
    const QString templateName = email.value(QStringLiteral("template")).toString();
    const QVariantList templateList = email.value(QStringLiteral("templates")).toList();
    if (templateName.isEmpty() && templateList.isEmpty()) {
        return ViewEmail::render(c);
    }

    QVariantList parts = email.value(QStringLiteral("parts")).toList();
    if (!templateList.isEmpty() && templateList.first().type() == QVariant::Hash) {
        // multipart API
        Q_FOREACH (const QVariant &part, templateList) {
            const QVariantHash partHash = part.toHash();
            MimePart *partObj = generatePart(c, d, partHash);
            parts.append(QVariant::fromValue(partObj));
        }

    } else if (!templateName.isEmpty()) {
        // single part API
        QVariantHash partArgs({
                                  {QStringLiteral("template"), templateName},

                              });
        auto contentTypeIt = email.constFind(QStringLiteral("content_type"));
        if (contentTypeIt != email.constEnd() && !contentTypeIt.value().toString().isEmpty()) {
            partArgs.insert(QStringLiteral("content_type"), contentTypeIt.value().toString());
        }
        MimePart *partObj = generatePart(c, d, partArgs);
        parts.append(QVariant::fromValue(partObj));
    }
    email.insert(QStringLiteral("parts"), parts);
    c->setStash(d->stashKey, email);

    return ViewEmail::render(c);
}
