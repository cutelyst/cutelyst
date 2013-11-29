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
#include "cutelyst.h"

#include <QCoreApplication>
#include <QStringBuilder>
#include <QRegularExpression>
#include <QSettings>
#include <QUuid>
#include <QDir>
#include <QDebug>

using namespace CutelystPlugin;

Session::Session(QObject *parent) :
    Plugin(parent)
{
}

bool Session::setup(CutelystApplication *app)
{
    connect(app, &CutelystApplication::afterDispatch,
            this, &Session::saveSession);
}

QVariant Session::value(Cutelyst *c, const QString &key, const QVariant &defaultValue)
{
    QVariant data = loadSession(c);
    if (data.isNull()) {
        return defaultValue;
    }

    QVariantHash session = data.value<QVariantHash>();
    return session.value(key, defaultValue);
}

void Session::setValue(Cutelyst *c, const QString &key, const QVariant &value)
{
    QVariantHash session = loadSession(c).value<QVariantHash>();
    session.insert(key, value);
    setPluginProperty(c, "sessionvalues", session);
    setPluginProperty(c, "sessionsave", true);
}

void Session::deleteValue(Cutelyst *c, const QString &key)
{
    setValue(c, key, QVariant());
}

bool Session::isValid(Cutelyst *c)
{
    return !loadSession(c).isNull();
}

QVariantHash Session::retrieveSession(const QString &sessionId) const
{
    qDebug() << Q_FUNC_INFO << filePath(sessionId);
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
    qDebug() << Q_FUNC_INFO << filePath(sessionId);
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

        qDebug() << Q_FUNC_INFO << "finished";
    }
}

void Session::saveSession(Cutelyst *c)
{
    qDebug() << Q_FUNC_INFO;
    if (!pluginProperty(c, "sessionsave").toBool()) {
        return;
    }

    QString sessionId = getSessionId(c);
    QNetworkCookie sessionCookie(sessionName().toLocal8Bit(),
                                 sessionId.toLocal8Bit());
    c->res()->addCookie(sessionCookie);
    persistSession(sessionId,
                   loadSession(c));
}

QString Session::sessionName() const
{
    return QCoreApplication::applicationName() % QLatin1String("_session");
}

QVariant Session::loadSession(Cutelyst *c)
{
    QVariant property = pluginProperty(c, "sessionvalues");
    if (!property.isNull()) {
        return property.value<QVariantHash>();
    }

    QString sessionid = getSessionId(c);
    if (!sessionid.isEmpty()) {
        QVariantHash session = retrieveSession(sessionid);
        setPluginProperty(c, "sessionvalues", session);
        return session;
    }
    return QVariant();
}

QString Session::generateSessionId() const
{
    return QUuid::createUuid().toString().remove(QRegularExpression("-|{|}"));
}

QString Session::getSessionId(Cutelyst *c) const
{
    QVariant property = c->property("Session::_sessionid");
    if (!property.isNull()) {
        return property.value<QString>();
    }

    QString sessionId;
    foreach (const QNetworkCookie &cookie, c->req()->cookies()) {
        if (cookie.name() == sessionName()) {
            sessionId = cookie.value();
            qDebug() << Q_FUNC_INFO << "Found sessionid" << sessionId << "in cookie";
        }
    }

    if (sessionId.isEmpty()) {
        sessionId = generateSessionId();
        qDebug() << Q_FUNC_INFO << "Created session" << sessionId;
    }
    c->setProperty("Session::_sessionid", sessionId);

    return sessionId;
}

QString Session::filePath(const QString &sessionId) const
{
    QString path = QDir::tempPath() % QLatin1Char('/') % QCoreApplication::applicationName();
    QDir dir;
    dir.mkpath(path);
    return path % QLatin1Char('/') % sessionId;
}
