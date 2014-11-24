/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#include "session_p.h"
#include "application.h"
#include "request.h"
#include "response.h"
#include "context.h"
#include "engine.h"

#include <QStringBuilder>
#include <QSettings>
#include <QUuid>
#include <QDir>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION, "cutelyst.plugin.session")

Session::Session(QObject *parent) :
    Plugin(parent),
    d_ptr(new SessionPrivate)
{
}

Cutelyst::Session::~Session()
{
    delete d_ptr;
}

bool Session::setup(Context *ctx)
{
    Q_D(Session);
    d->ctx = ctx;
    d->sessionName = ctx->app()->applicationName() % QStringLiteral("_session");
    connect(ctx, &Context::afterDispatch,
            this, &Session::saveSession);
    return true;
}

QVariant Session::value(const QString &key, const QVariant &defaultValue)
{
    QVariant data = loadSession();
    if (data.isNull()) {
        return defaultValue;
    }

    QVariantHash session = data.value<QVariantHash>();
    return session.value(key, defaultValue);
}

void Session::setValue(const QString &key, const QVariant &value)
{
    Q_D(Session);
    QVariantHash session = loadSession().value<QVariantHash>();
    session.insert(key, value);
    setPluginProperty(d->ctx, QStringLiteral("sessionvalues"), session);
    setPluginProperty(d->ctx, QStringLiteral("sessionsave"), true);
}

void Session::deleteValue(const QString &key)
{
    setValue(key, QVariant());
}

bool Session::isValid()
{
    return !loadSession().isNull();
}

QVariantHash Session::retrieveSession(const QString &sessionId) const
{
    QVariantHash ret;
    QSettings settings(filePath(sessionId), QSettings::IniFormat);
    settings.beginGroup(QLatin1String("Data"));
    Q_FOREACH (const QString &key, settings.allKeys()) {
        ret.insert(key, settings.value(key));
    }
    settings.endGroup();
    return ret;
}

void Session::persistSession(const QString &sessionId, const QVariant &data) const
{
    QSettings settings(filePath(sessionId), QSettings::IniFormat);
    if (data.isNull()) {
        settings.clear();
    } else {
        settings.beginGroup(QLatin1String("Data"));
        QVariantHash hash = data.value<QVariantHash>();
        QVariantHash::ConstIterator it = hash.constBegin();
        while (it != hash.constEnd()) {
            if (it.value().isNull()) {
                settings.remove(it.key());
            } else {
                settings.setValue(it.key(), it.value());
            }
            ++it;

        }
        settings.endGroup();
    }
}

void Session::saveSession()
{
    Q_D(Session);
    if (!pluginProperty(d->ctx, "sessionsave").toBool()) {
        return;
    }

    QString sessionId = getSessionId();
    QNetworkCookie sessionCookie(d->sessionName.toLocal8Bit(),
                                 sessionId.toLocal8Bit());
    d->ctx->res()->addCookie(sessionCookie);
    persistSession(sessionId,
                   loadSession());
}

QVariant Session::loadSession()
{
    Q_D(Session);
    QVariant property = pluginProperty(d->ctx, "sessionvalues");
    if (!property.isNull()) {
        return property.value<QVariantHash>();
    }

    QString sessionid = getSessionId();
    if (!sessionid.isEmpty()) {
        QVariantHash session = retrieveSession(sessionid);
        setPluginProperty(d->ctx, "sessionvalues", session);
        return session;
    }
    return QVariant();
}

QString Session::generateSessionId() const
{
    Q_D(const Session);
    QRegularExpression re = d->removeRE; // Thread-safe
    return QUuid::createUuid().toString().remove(re);
}

QString Session::getSessionId() const
{
    Q_D(const Session);
    QVariant property = d->ctx->property("Session/_sessionid");
    if (!property.isNull()) {
        return property.value<QString>();
    }

    QString sessionId;
    Q_FOREACH (const QNetworkCookie &cookie, d->ctx->req()->cookies()) {
        if (cookie.name() == d->sessionName) {
            sessionId = cookie.value();
            qCDebug(C_SESSION) << "Found sessionid" << sessionId << "in cookie";
        }
    }

    if (sessionId.isEmpty()) {
        sessionId = generateSessionId();
        qCDebug(C_SESSION) << "Created session" << sessionId;
    }
    d->ctx->setProperty("Session/_sessionid", sessionId);

    return sessionId;
}

QString Session::filePath(const QString &sessionId) const
{
    Q_D(const Session);
    QString path = QDir::tempPath() % QLatin1Char('/') % d->ctx->app()->applicationName();
    QDir dir;
    if (!dir.mkpath(path)) {
        qCWarning(C_SESSION) << "Failed to create path for session object" << path;
    }
    return path % QLatin1Char('/') % sessionId;
}
