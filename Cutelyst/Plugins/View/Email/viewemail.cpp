/*
 * Copyright (C) 2015-2018 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "viewemail_p.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/response.h>

#include <QtCore/QLoggingCategory>

#include <SimpleMail/mimemessage.h>
#include <SimpleMail/emailaddress.h>
#include <SimpleMail/mimetext.h>

Q_LOGGING_CATEGORY(CUTELYST_VIEW_EMAIL, "cutelyst.view.email", QtWarningMsg)

using namespace Cutelyst;
using namespace SimpleMail;

ViewEmail::ViewEmail(QObject *parent, const QString &name) : View(new ViewEmailPrivate, parent, name)
{
    initSender();
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
    Q_EMIT changed();
}

QByteArray ViewEmail::defaultContentType() const
{
    Q_D(const ViewEmail);
    return d->defaultContentType;
}

void ViewEmail::setDefaultContentType(const QByteArray &contentType)
{
    Q_D(ViewEmail);
    d->defaultContentType = contentType;
    Q_EMIT changed();
}

QByteArray ViewEmail::defaultCharset() const
{
    Q_D(const ViewEmail);
    return d->defaultCharset;
}

void ViewEmail::setDefaultCharset(const QByteArray &charset)
{
    Q_D(ViewEmail);
    d->defaultCharset = charset;
    Q_EMIT changed();
}

QByteArray ViewEmail::defaultEncoding() const
{
    Q_D(const ViewEmail);
    return d->defaultEncoding;
}

void ViewEmail::setDefaultEncoding(const QByteArray &encoding)
{
    Q_D(ViewEmail);
    d->defaultEncoding = encoding;
    Q_EMIT changed();
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

ViewEmail::ConnectionType ViewEmail::senderConnectionType() const
{
    Q_D(const ViewEmail);
    return static_cast<ViewEmail::ConnectionType>(d->sender->connectionType());
}

void ViewEmail::setSenderConnectionType(ViewEmail::ConnectionType ct)
{
    Q_D(ViewEmail);
    d->sender->setConnectionType(static_cast<Sender::ConnectionType>(ct));
}

ViewEmail::AuthMethod ViewEmail::senderAuthMethod() const
{
    Q_D(const ViewEmail);
    return static_cast<ViewEmail::AuthMethod>(d->sender->authMethod());
}

void ViewEmail::setSenderAuthMethod(ViewEmail::AuthMethod method)
{
    Q_D(ViewEmail);
    d->sender->setAuthMethod(static_cast<Sender::AuthMethod>(method));
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
    QByteArray ret;
    QVariantHash email = c->stash(d->stashKey).toHash();
    if (email.isEmpty()) {
        c->error(QStringLiteral("Cannot render template, template name or template stash key not defined"));
        return ret;
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
        c->error(QStringLiteral("Can't send email without parts or body, check stash"));
        return ret;
    }

    if (!parts.isNull()) {
        const QVariantList partsVariant = parts.toList();
        for (const QVariant &part : partsVariant) {
            MimePart *mime = part.value<MimePart*>();
            if (mime) {
                message.addPart(mime);
            } else {
                qCCritical(CUTELYST_VIEW_EMAIL) << "Failed to cast MimePart";
            }
        }

        auto contentTypeIt = email.constFind(QStringLiteral("content_type"));
        if (contentTypeIt != email.constEnd()
                && !contentTypeIt.value().isNull()
                && !contentTypeIt.value().toString().isEmpty()) {
            const QByteArray contentType = contentTypeIt.value().toString().toLatin1();
            qCDebug(CUTELYST_VIEW_EMAIL) << "Using specified content_type" << contentType;
            message.getContent().setContentType(contentType);
        } else if (!d->defaultContentType.isEmpty()) {
            qCDebug(CUTELYST_VIEW_EMAIL) << "Using default content_type" << d->defaultContentType;
            message.getContent().setContentType(d->defaultContentType);
        }
    } else {
        MimeText *part = new MimeText(body.toString());
        d->setupAttributes(part, email);
        message.setContent(part);
    }

    if (!d->sender->sendMail(message)) {
        c->error(QString::fromLatin1(d->sender->responseText()));
        return ret;
    }

    return ret;
}

ViewEmail::ViewEmail(ViewEmailPrivate *d, QObject *parent, const QString &name) : View(parent, name)
{
    d_ptr = d;

    initSender();
}

void ViewEmail::initSender()
{
    Q_D(ViewEmail);
    d->sender = new Sender(this);

    QVariantHash config;
    const auto app = qobject_cast<Application *>(parent());
    if (app) {
        config = app->config(QStringLiteral("VIEW_EMAIL")).toHash();
    }

    d->stashKey = config.value(QStringLiteral("stash_key"), QStringLiteral("email")).toString();

    if (!config.value(QStringLiteral("sender_host")).isNull()) {
        d->sender->setHost(config.value(QStringLiteral("sender_host")).toString());
    }
    if (!config.value(QStringLiteral("sender_port")).isNull()) {
        d->sender->setPort(config.value(QStringLiteral("sender_port")).toInt());
    }
    if (!config.value(QStringLiteral("sender_username")).isNull()) {
        d->sender->setUser(config.value(QStringLiteral("sender_username")).toString());
    }
    if (!config.value(QStringLiteral("sender_password")).isNull()) {
        d->sender->setPassword(config.value(QStringLiteral("sender_password")).toString());
    }
}


void ViewEmailPrivate::setupAttributes(MimePart *part, const QVariantHash &attrs) const
{
    auto contentTypeIt = attrs.constFind(QStringLiteral("content_type"));
    if (contentTypeIt != attrs.constEnd()
            && !contentTypeIt.value().isNull()
            && !contentTypeIt.value().toString().isEmpty()) {
        const QByteArray contentType = contentTypeIt.value().toString().toLatin1();
        qCDebug(CUTELYST_VIEW_EMAIL) << "Using specified content_type" << contentType;
        part->setContentType(contentType);
    } else if (!defaultContentType.isEmpty()) {
        qCDebug(CUTELYST_VIEW_EMAIL) << "Using default content_type" << defaultContentType;
        part->setContentType(defaultContentType);
    }

    auto charsetIt = attrs.constFind(QStringLiteral("charset"));
    if (charsetIt != attrs.constEnd()
            && !charsetIt.value().isNull()
            && !charsetIt.value().toString().isEmpty()) {
        const QByteArray charset = charsetIt.value().toString().toLatin1();
        qCDebug(CUTELYST_VIEW_EMAIL) << "Using specified charset" << charset;
        part->setCharset(charset);
    } else if (!defaultCharset.isEmpty()) {
        qCDebug(CUTELYST_VIEW_EMAIL) << "Using default charset" << defaultCharset;
        part->setCharset(defaultCharset);
    }

    auto encodingIt = attrs.constFind(QStringLiteral("encoding"));
    if (encodingIt != attrs.constEnd()
            && !encodingIt.value().isNull()
            && !encodingIt.value().toString().isEmpty()) {
        const QByteArray encoding = encodingIt.value().toString().toLatin1();
        qCDebug(CUTELYST_VIEW_EMAIL) << "Using specified encoding" << encoding;
        setupEncoding(part, encoding);
    } else if (!defaultEncoding.isEmpty()) {
        qCDebug(CUTELYST_VIEW_EMAIL) << "Using default charset" << defaultEncoding;
        setupEncoding(part, defaultEncoding);
    }
}

void ViewEmailPrivate::setupEncoding(MimePart *part, const QByteArray &encoding) const
{
    if (encoding == "7bit") {
        part->setEncoding(MimePart::_7Bit);
    } else if (encoding == "8bit") {
        part->setEncoding(MimePart::_8Bit);
    } else if (encoding == "base64") {
        part->setEncoding(MimePart::Base64);
    } else if (encoding == "quoted-printable") {
        part->setEncoding(MimePart::QuotedPrintable);
    } else {
        qCCritical(CUTELYST_VIEW_EMAIL) << "Unknown encoding" << encoding;
    }
}

#include "moc_viewemail.cpp"
