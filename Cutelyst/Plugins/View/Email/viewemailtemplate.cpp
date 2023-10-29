/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "viewemailtemplate_p.h"

#include <Cutelyst/Context>
#include <SimpleMail/emailaddress.h>
#include <SimpleMail/mimemessage.h>
#include <SimpleMail/mimetext.h>

#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_VIEW_EMAILTEMPLATE, "cutelyst.view.emailtemplate", QtWarningMsg)

using namespace Cutelyst;

ViewEmailTemplate::ViewEmailTemplate(QObject *parent, const QString &name)
    : ViewEmail(new ViewEmailTemplatePrivate, parent, name)
{
    Q_D(ViewEmailTemplate);

    d->defaultContentType = QByteArrayLiteral("text/html");
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
    Q_EMIT changedProp();
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
    Q_EMIT changedProp();
}

MimePart *generatePart(Context *c, const ViewEmailTemplatePrivate *d, const QVariantHash &partHash)
{
    const QString defaultView = d->defaultView;

    View *view  = nullptr;
    auto viewIt = partHash.constFind(QStringLiteral("view"));
    if (viewIt != partHash.constEnd() && !viewIt.value().toString().isEmpty()) {
        // use the view specified for the email part
        const QString viewString = viewIt.value().toString();
        qCDebug(CUTELYST_VIEW_EMAILTEMPLATE)
            << "Using specified view" << viewString << "for rendering.";
        view = c->view(viewString);
    } else if (!defaultView.isEmpty()) {
        // if none specified use the configured default view
        qCDebug(CUTELYST_VIEW_EMAILTEMPLATE)
            << "Using default view" << defaultView << "for rendering.";
        view = c->view(defaultView);
    } else {
        // else fallback to Cutelysts default view
        qCDebug(CUTELYST_VIEW_EMAILTEMPLATE) << "Using Cutelysts default view for rendering.";
        view = c->view(QString());
    }

    // validate the per template view
    if (!view) {
        c->appendError(QStringLiteral("Could not find a view to render"));
        return nullptr;
    }

    QString templateString = partHash.value(QStringLiteral("template")).toString();
    ;
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

    QByteArray ret;
    QVariantHash email              = c->stash(d->stashKey).toHash();
    const QString templateName      = email.value(QStringLiteral("template")).toString();
    const QVariantList templateList = email.value(QStringLiteral("templates")).toList();
    if (templateName.isEmpty() && templateList.isEmpty()) {
        ret = ViewEmail::render(c);
        return ret;
    }

    QVariantList parts = email.value(QStringLiteral("parts")).toList();
    if (!templateList.isEmpty() && templateList.first().typeId() == QMetaType::QVariantHash) {
        // multipart API
        for (const QVariant &part : templateList) {
            const QVariantHash partHash = part.toHash();
            MimePart *partObj           = generatePart(c, d, partHash);
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

    ret = ViewEmail::render(c);
    return ret;
}

#include "moc_viewemailtemplate.cpp"
