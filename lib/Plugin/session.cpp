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
#include "cutelystapplication.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"
#include "context.h"

#include <QCoreApplication>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QSettings>
#include <QUuid>
#include <QDir>
#include <QDebug>

using namespace Cutelyst;
using namespace CutelystPlugin;

Session::Session(QObject *parent) :
    Plugin(parent)
{
}

bool Session::setup(Application *app)
{
    connect(app, &Application::afterDispatch,
            this, &Session::saveSession);
}

QVariant Session::value(Context *ctx, const QString &key, const QVariant &defaultValue)
{
    QVariant data = loadSession(ctx);
    if (data.isNull()) {
        return defaultValue;
    }

    QVariantHash session = data.value<QVariantHash>();
    return session.value(key, defaultValue);
}

void Session::setValue(Context *ctx, const QString &key, const QVariant &value)
{
    QVariantHash session = loadSession(ctx).value<QVariantHash>();
    session.insert(key, value);
    setPluginProperty(ctx, "sessionvalues", session);
    setPluginProperty(ctx, "sessionsave", true);
}

void Session::deleteValue(Context *ctx, const QString &key)
{
    setValue(ctx, key, QVariant());
}

bool Session::isValid(Context *ctx)
{
    return !loadSession(ctx).isNull();
}

QVariantHash Session::retrieveSession(const QString &sessionId) const
{
    QVariantHash ret;
    QSettings settings(filePath(sessionId), QSettings::IniFormat);
    settings.beginGroup(QLatin1String("Data"));
    foreach (const QString &key, settings.allKeys()) {
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

void Session::saveSession(Context *ctx)
{
    if (!pluginProperty(ctx, "sessionsave").toBool()) {
        return;
    }

    QString sessionId = getSessionId(ctx);
    QNetworkCookie sessionCookie(sessionName().toLocal8Bit(),
                                 sessionId.toLocal8Bit());
    ctx->res()->addCookie(sessionCookie);
    persistSession(sessionId,
                   loadSession(ctx));
}

QString Session::sessionName() const
{
    return QCoreApplication::applicationName() % QLatin1String("_session");
}

QVariant Session::loadSession(Context *ctx)
{
    QVariant property = pluginProperty(ctx, "sessionvalues");
    if (!property.isNull()) {
        return property.value<QVariantHash>();
    }

    QString sessionid = getSessionId(ctx);
    if (!sessionid.isEmpty()) {
        QVariantHash session = retrieveSession(sessionid);
        setPluginProperty(ctx, "sessionvalues", session);
        return session;
    }
    return QVariant();
}

QString Session::generateSessionId() const
{
    return QUuid::createUuid().toString().remove(QRegularExpression("-|{|}"));
}

QString Session::getSessionId(Context *ctx) const
{
    QVariant property = ctx->property("Session::_sessionid");
    if (!property.isNull()) {
        return property.value<QString>();
    }

    QString sessionId;
    foreach (const QNetworkCookie &cookie, ctx->req()->cookies()) {
        if (cookie.name() == sessionName()) {
            sessionId = cookie.value();
            qDebug() << "Found sessionid" << sessionId << "in cookie";
        }
    }

    if (sessionId.isEmpty()) {
        sessionId = generateSessionId();
        qDebug() << "Created session" << sessionId;
    }
    ctx->setProperty("Session::_sessionid", sessionId);

    return sessionId;
}

QString Session::filePath(const QString &sessionId) const
{
    QString path = QDir::tempPath() % QLatin1Char('/') % QCoreApplication::applicationName();
    QDir dir;
    dir.mkpath(path);
    return path % QLatin1Char('/') % sessionId;
}
