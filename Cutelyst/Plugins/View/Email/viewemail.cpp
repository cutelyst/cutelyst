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

#include "viewemail_p.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>

#include <QtCore/QLoggingCategory>

#include <simplemail-qt5/SimpleMail/mimemessage.h>
#include <simplemail-qt5/SimpleMail/emailaddress.h>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(CUTELYST_VIEW_EMAIL, "cutelyst.view.email")

ViewEmail::ViewEmail(QObject *parent, const QString &name) : View(parent, name)
  , d_ptr(new ViewEmailPrivate)
{
    Q_D(ViewEmail);

    Application *app = qobject_cast<Application *>(parent);
    if (app) {
        d->stashKey = app->config(QLatin1String("VIEW_EMAIL_STASH_KEY"), QStringLiteral("email")).toString();
    }
}

ViewEmail::~ViewEmail()
{
    delete d_ptr;
}

bool ViewEmail::render(Context *c) const
{
    Q_D(const ViewEmail);

    QVariantHash email = c->stash(d->stashKey).toHash();
    if (email.isEmpty()) {
        qCCritical(CUTELYST_VIEW_EMAIL) << "Cannot render template, template name or template stash key not defined";
        return false;
    }

    MimeMessage message;

    QVariant value;
    value = email.value(QStringLiteral("to"));
    if (value.type() == QVariant::String && !value.toString().isEmpty()) {
        message.addTo(value.toString());
    }

    value = email.value(QStringLiteral("cc"));
    if (value.type() == QVariant::String && !value.toString().isEmpty()) {
        message.addCc(value.toString());
    }

    value = email.value(QStringLiteral("from"));
    if (value.type() == QVariant::String && !value.toString().isEmpty()) {
        message.setSender(value.toString());
    }

    value = email.value(QStringLiteral("subject"));
    if (value.type() == QVariant::String && !value.toString().isEmpty()) {
        message.setSubject(value.toString());
    }

    QVariant body = email.value(QStringLiteral("body"));
    QVariant parts = email.value(QStringLiteral("parts"));
    if (body.isNull() && parts.isNull()) {
        qCCritical(CUTELYST_VIEW_EMAIL) << "Can't send email without parts or body, check stash";
        return false;
    }




    return true;
}
