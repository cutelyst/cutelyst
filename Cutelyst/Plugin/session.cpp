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

#include "session.h"
#include "application.h"
#include "request.h"
#include "response.h"
#include "context.h"
#include "engine.h"

#include <QStringBuilder>
#include <QRegularExpression>
#include <QSettings>
#include <QUuid>
#include <QDir>
#include <QLoggingCategory>

using namespace Cutelyst;
using namespace Plugin;

Q_LOGGING_CATEGORY(C_SESSION, "cutelyst.plugin.session")

Session::Session(QObject *parent) :
    AbstractPlugin(parent)
{
}

bool Session::setup(Context *ctx)
{
    m_ctx = ctx;
    m_sessionName = ctx->engine()->app()->applicationName() % QLatin1String("_session");
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
    QVariantHash session = loadSession().value<QVariantHash>();
    session.insert(key, value);
    setPluginProperty(m_ctx, "sessionvalues", session);
    setPluginProperty(m_ctx, "sessionsave", true);
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
    if (!pluginProperty(m_ctx, "sessionsave").toBool()) {
        return;
    }

    QString sessionId = getSessionId();
    QNetworkCookie sessionCookie(m_sessionName.toLocal8Bit(),
                                 sessionId.toLocal8Bit());
    m_ctx->res()->addCookie(sessionCookie);
    persistSession(sessionId,
                   loadSession());
}

QVariant Session::loadSession()
{
    QVariant property = pluginProperty(m_ctx, "sessionvalues");
    if (!property.isNull()) {
        return property.value<QVariantHash>();
    }

    QString sessionid = getSessionId();
    if (!sessionid.isEmpty()) {
        QVariantHash session = retrieveSession(sessionid);
        setPluginProperty(m_ctx, "sessionvalues", session);
        return session;
    }
    return QVariant();
}

QString Session::generateSessionId() const
{
    return QUuid::createUuid().toString().remove(QRegularExpression("-|{|}"));
}

QString Session::getSessionId() const
{
    QVariant property = m_ctx->property("Session/_sessionid");
    if (!property.isNull()) {
        return property.value<QString>();
    }

    QString sessionId;
    Q_FOREACH (const QNetworkCookie &cookie, m_ctx->req()->cookies()) {
        if (cookie.name() == m_sessionName) {
            sessionId = cookie.value();
            qCDebug(C_SESSION) << "Found sessionid" << sessionId << "in cookie";
        }
    }

    if (sessionId.isEmpty()) {
        sessionId = generateSessionId();
        qCDebug(C_SESSION) << "Created session" << sessionId;
    }
    m_ctx->setProperty("Session/_sessionid", sessionId);

    return sessionId;
}

QString Session::filePath(const QString &sessionId) const
{
    QString path = QDir::tempPath() % QLatin1Char('/') % m_ctx->engine()->app()->applicationName();
    QDir dir;
    if (!dir.mkpath(path)) {
        qCWarning(C_SESSION) << "Failed to create path for session object" << path;
    }
    return path % QLatin1Char('/') % sessionId;
}
