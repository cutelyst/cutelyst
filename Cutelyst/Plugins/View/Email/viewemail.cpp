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

#include <SimpleMail/mimemessage.h>
#include <SimpleMail/emailaddress.h>
#include <SimpleMail/mimetext.h>

Q_LOGGING_CATEGORY(CUTELYST_VIEW_EMAIL, "cutelyst.view.email")

using namespace Cutelyst;
using namespace SimpleMail;

ViewEmail::ViewEmail(QObject *parent, const QString &name) : View(parent, name)
  , d_ptr(new ViewEmailPrivate)
{
    Q_D(ViewEmail);
    d->sender = new Sender(this);

    QVariantHash config;
    Application *app = qobject_cast<Application *>(parent);
    if (app) {
        config = app->config(QLatin1String("VIEW_EMAIL")).toHash();
    }

    d->stashKey = config.value(QLatin1String("stash_key"), QStringLiteral("email")).toString();

    if (!config.value(QLatin1String("sender_host")).isNull()) {
        d->sender->setHost(config.value(QLatin1String("sender_host")).toString());
    }
    if (!config.value(QLatin1String("sender_port")).isNull()) {
        d->sender->setPort(config.value(QLatin1String("sender_port")).toInt());
    }
    if (!config.value(QLatin1String("sender_username")).isNull()) {
        d->sender->setUser(config.value(QLatin1String("sender_username")).toString());
    }
    if (!config.value(QLatin1String("sender_password")).isNull()) {
        d->sender->setPassword(config.value(QLatin1String("sender_password")).toString());
    }
}

ViewEmail::~ViewEmail()
{
    delete d_ptr;
}

QString ViewEmail::stashKey() const
{
    Q_D(const ViewEmail);
    return d->stashKey;
}

void ViewEmail::setStashKey(const QString &stashKey)
{
    Q_D(ViewEmail);
    d->stashKey = stashKey;
}

QString ViewEmail::defaultContentType() const
{
    Q_D(const ViewEmail);
    return d->defaultContentType;
}

void ViewEmail::setDefaultContentType(const QString &contentType)
{
    Q_D(ViewEmail);
    d->defaultContentType = contentType;
}

QString ViewEmail::defaultCharset() const
{
    Q_D(const ViewEmail);
    return d->defaultCharset;
}

void ViewEmail::setDefaultCharset(const QString &charset)
{
    Q_D(ViewEmail);
    d->defaultCharset = charset;
}

QString ViewEmail::senderHost() const
{
    Q_D(const ViewEmail);
    return d->sender->host();
}

void ViewEmail::setSenderHost(const QString &host)
{
    Q_D(ViewEmail);
    d->sender->setHost(host);
}

int ViewEmail::senderPort() const
{
    Q_D(const ViewEmail);
    return d->sender->port();
}

void ViewEmail::setSenderPort(int port)
{
    Q_D(ViewEmail);
    d->sender->setPort(port);
}

QString ViewEmail::senderUser() const
{
    Q_D(const ViewEmail);
    return d->sender->user();
}

void ViewEmail::setSenderUser(const QString &user)
{
    Q_D(ViewEmail);
    d->sender->setUser(user);
}

QString ViewEmail::senderPassword() const
{
    Q_D(const ViewEmail);
    return d->sender->password();
}

void ViewEmail::setSenderPassword(const QString &password)
{
    Q_D(ViewEmail);
    d->sender->setPassword(password);
}

QByteArray ViewEmail::render(Context *c) const
{
    Q_D(const ViewEmail);

    QVariantHash email = c->stash(d->stashKey).toHash();
    if (email.isEmpty()) {
        c->error(QLatin1String("Cannot render template, template name or template stash key not defined"));
        return QByteArray();
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
        c->error(QLatin1String("Can't send email without parts or body, check stash"));
        return QByteArray();
    }

    if (!parts.isNull()) {
        QVariantList partsVariant = parts.toList();
        Q_FOREACH (const QVariant &part, partsVariant) {
            message.addPart(part.value<MimePart*>());
        }
    } else if (!body.isNull()) {
        message.setContent(new MimeText(body.toString()));
    }

    if (!d->sender->sendMail(message)) {
        c->error(QString::fromLatin1(d->sender->responseText()));
        return QByteArray();
    }

    return QByteArray();
}

ViewEmail::ViewEmail(ViewEmailPrivate *d, QObject *parent, const QString &name) : View(parent, name)
{
    d_ptr = d;
}
