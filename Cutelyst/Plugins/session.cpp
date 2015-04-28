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

Session::Session(Application *parent) : Plugin(parent)
  , d_ptr(new SessionPrivate)
{
}

Cutelyst::Session::~Session()
{
    delete d_ptr;
}

bool Session::setup(Application *app)
{
    Q_D(Session);
    d->sessionName = QCoreApplication::applicationName() % QStringLiteral("_session");
    connect(app, &Application::afterDispatch,
            this, &Session::saveSession);
    return true;
}

QVariant Session::value(Cutelyst::Context *c, const QString &key, const QVariant &defaultValue)
{
    QVariant data = loadSession(c);
    if (data.isNull()) {
        return defaultValue;
    }

    QVariantHash session = data.value<QVariantHash>();
    return session.value(key, defaultValue);
}

void Session::setValue(Cutelyst::Context *c, const QString &key, const QVariant &value)
{
    QVariantHash session = loadSession(c).value<QVariantHash>();
    session.insert(key, value);
    setPluginProperty(c, QStringLiteral("sessionvalues"), session);
    setPluginProperty(c, QStringLiteral("sessionsave"), true);
}

void Session::deleteValue(Context *c, const QString &key)
{
    setValue(c, key, QVariant());
}

bool Session::isValid(Cutelyst::Context *c)
{
    return !loadSession(c).isNull();
}

QVariantHash Session::retrieveSession(const QString &sessionId) const
{
    QVariantHash ret;
    QSettings settings(SessionPrivate::filePath(sessionId), QSettings::IniFormat);
    settings.beginGroup(QLatin1String("Data"));
    Q_FOREACH (const QString &key, settings.allKeys()) {
        ret.insert(key, settings.value(key));
    }
    settings.endGroup();
    return ret;
}

void Session::persistSession(const QString &sessionId, const QVariant &data) const
{
    QSettings settings(SessionPrivate::filePath(sessionId), QSettings::IniFormat);
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

void Session::saveSession(Cutelyst::Context *c)
{
    Q_D(Session);
    if (!pluginProperty(c, "sessionsave").toBool()) {
        return;
    }

    QString sessionId = d->getSessionId(c, true);
    QNetworkCookie sessionCookie(d->sessionName.toLocal8Bit(),
                                 sessionId.toLocal8Bit());
    c->res()->addCookie(sessionCookie);
    persistSession(sessionId,
                   loadSession(c));
}

QVariant Session::loadSession(Cutelyst::Context *c)
{
    Q_D(Session);
    QVariant property = pluginProperty(c, "sessionvalues");
    if (!property.isNull()) {
        return property.value<QVariantHash>();
    }

    QString sessionid = d->getSessionId(c, false);
    if (!sessionid.isEmpty()) {
        QVariantHash session = retrieveSession(sessionid);
        setPluginProperty(c, "sessionvalues", session);
        return session;
    }
    return QVariant();
}

QString SessionPrivate::filePath(const QString &sessionId)
{
    QString path = QDir::tempPath() % QLatin1Char('/') % QCoreApplication::applicationName();
    QDir dir;
    if (!dir.mkpath(path)) {
        qCWarning(C_SESSION) << "Failed to create path for session object" << path;
    }
    return path % QLatin1Char('/') % sessionId;
}

QString SessionPrivate::generateSessionId() const
{
    QRegularExpression re = removeRE; // Thread-safe
    return QUuid::createUuid().toString().remove(re);
}

QString SessionPrivate::getSessionId(Context *c, bool create) const
{
    QVariant property = c->property("Session/_sessionid");
    if (!property.isNull()) {
        return property.toString();
    }

    QString sessionId;
    Q_FOREACH (const QNetworkCookie &cookie, c->req()->cookies()) {
        if (cookie.name() == sessionName) {
            sessionId = cookie.value();
            qCDebug(C_SESSION) << "Found sessionid" << sessionId << "in cookie";
            break;
        }
    }

    if (sessionId.isEmpty()) {
        if (create) {
            sessionId = generateSessionId();
            qCDebug(C_SESSION) << "Created session" << sessionId;
            c->setProperty("Session/_sessionid", sessionId);
        }
    } else {
        c->setProperty("Session/_sessionid", sessionId);
    }

    return sessionId;
}
