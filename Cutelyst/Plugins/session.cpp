/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include <QtCore/QStringBuilder>
#include <QtCore/QSettings>
#include <QtCore/QUuid>
#include <QtCore/QDir>
#include <QtCore/QLoggingCategory>
#include <QtCore/QCoreApplication>
#include <QtNetwork/QNetworkCookie>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION, "cutelyst.plugin.session")

#define SESSION_VALUES "__session_values"
#define SESSION_EXPIRES "__session_expires"
#define SESSION_SAVE "__session_save"
#define SESSION_ID "__session_id"
#define SESSION_DELETED_ID "__session_deleted_id"

Session::Session(Application *parent) : Plugin(parent)
  , d_ptr(new SessionPrivate)
{
    d_ptr->q_ptr = this;
}

Cutelyst::Session::~Session()
{
    delete d_ptr;
}

bool Session::setup(Application *app)
{
    Q_D(Session);
    d->sessionName = QCoreApplication::applicationName() % QStringLiteral("_session");

    QVariantHash config = app->config("Session_Plugin").toHash();
    d->sessionExpires = config.value("expires", 7200).toULongLong();

    connect(app, &Application::afterDispatch, d, &SessionPrivate::saveSession);
    return true;
}

QString Session::id(Cutelyst::Context *c)
{
    Session *session = c->plugin<Session*>();
    if (!session) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return QString();
    }

    return session->d_ptr->getSessionId(c, session->d_ptr->sessionName);
}

quint64 Session::expires(Context *c)
{
    SessionPrivate::loadSession(c, false);
    return c->property(SESSION_EXPIRES).toULongLong();
}

void Session::setExpires(Context *c, quint64 expires)
{

}

QVariant Session::value(Cutelyst::Context *c, const QString &key, const QVariant &defaultValue)
{
    const QVariant &data = SessionPrivate::loadSession(c, false);
    if (data.isNull()) {
        return defaultValue;
    }

    const QVariantHash &session = data.toHash();
    return session.value(key, defaultValue);
}

void Session::setValue(Cutelyst::Context *c, const QString &key, const QVariant &value)
{
    QVariantHash session = SessionPrivate::loadSession(c, true).toHash();
    if (value.isNull()) {
        session.remove(key);
    } else {
        session.insert(key, value);
    }
    c->setProperty(SESSION_VALUES, session);
    c->setProperty(SESSION_SAVE, true);
}

void Session::deleteValue(Context *c, const QString &key)
{
    setValue(c, key, QVariant());
}

bool Session::isValid(Cutelyst::Context *c)
{
    return !SessionPrivate::loadSession(c, false).isNull();
}

QVariantHash Session::retrieveSession(const QString &sessionId, quint64 &expires) const
{
    QVariantHash ret;
    const QString &sessionFile = SessionPrivate::filePath(sessionId);
    expires = 0;
    if (QFileInfo::exists(sessionFile)) {
        qCDebug(C_SESSION) << "Retrieving session values from" << sessionFile;

        QSettings settings(sessionFile, QSettings::IniFormat);
        expires = settings.value(QLatin1String("expires")).toULongLong();

        settings.beginGroup(QLatin1String("Data"));
        Q_FOREACH (const QString &key, settings.allKeys()) {
            ret.insert(key, settings.value(key));
        }
        settings.endGroup();
    }
    return ret;
}

void Session::persistSession(const QString &sessionId, const QVariant &data, quint64 expires) const
{
    const QString &sessionFile = SessionPrivate::filePath(sessionId);
    const QVariantHash &hash = data.toHash();
    if (hash.isEmpty()) {
        qCDebug(C_SESSION) << "Clearing session values on" << sessionFile;
        bool ret = QFile::remove(sessionFile);
        if (!ret) {
            QSettings settings(sessionFile, QSettings::IniFormat);
            settings.clear();
        }
    } else {
        qCDebug(C_SESSION) << "Persisting session values to" << sessionFile;
        QSettings settings(sessionFile, QSettings::IniFormat);
        settings.setValue(QStringLiteral("expires"), expires);

        settings.beginGroup(QStringLiteral("Data"));
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

QString SessionPrivate::filePath(const QString &sessionId)
{
    QString path = QDir::tempPath() % QLatin1Char('/') % QCoreApplication::applicationName();
    QDir dir;
    if (!dir.mkpath(path)) {
        qCWarning(C_SESSION) << "Failed to create path for session object" << path;
    }
    return path % QLatin1Char('/') % sessionId;
}

QString SessionPrivate::generateSessionId()
{
    return QString::fromLatin1(QUuid::createUuid().toRfc4122().toHex());
}

QString SessionPrivate::getSessionId(Context *c, const QString &sessionName)
{
    bool deleted = !c->property(SESSION_DELETED_ID).isNull();

    QString sessionId;
    if (!deleted) {
        const QVariant &property = c->property(SESSION_ID);
        if (!property.isNull()) {
            return property.toString();
        }

        Q_FOREACH (const QNetworkCookie &cookie, c->req()->cookies()) {
            if (cookie.name() == sessionName) {
                sessionId = cookie.value();
                qCDebug(C_SESSION) << "Found sessionid" << sessionId << "in cookie";
                break;
            }
        }
        c->setProperty(SESSION_ID, sessionId);
    }

    return sessionId;
}

QString SessionPrivate::createSessionId(Context *c, quint64 expires)
{
    QString sessionId = generateSessionId();
    qCDebug(C_SESSION) << "Created session" << sessionId;
    c->setProperty(SESSION_ID, sessionId);
    c->setProperty(SESSION_DELETED_ID, QVariant());
    c->setProperty(SESSION_EXPIRES, (QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000) + expires);
    return sessionId;
}

void SessionPrivate::saveSession(Context *c)
{
    if (!c->property(SESSION_SAVE).toBool()) {
        return;
    }

    Session *session = c->plugin<Session*>();
    if (!session) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return;
    }

    const QString &_sessionName = session->d_ptr->sessionName;
    const QString &sessionId = c->property(SESSION_ID).toString();

    bool deleted = !c->property(SESSION_DELETED_ID).isNull();
    QDateTime expiresDt;
    if (deleted) {
        expiresDt = QDateTime::currentDateTimeUtc();
    } else {
        expiresDt = QDateTime::currentDateTimeUtc().addSecs(session->d_ptr->sessionExpires);
        const QVariant &sessionData = c->property(SESSION_VALUES);
        session->persistSession(sessionId, sessionData, expiresDt.toMSecsSinceEpoch() / 1000);
    }

    QNetworkCookie sessionCookie(_sessionName.toLatin1(), sessionId.toLatin1());
    sessionCookie.setPath(QStringLiteral("/"));
    sessionCookie.setExpirationDate(expiresDt);
    sessionCookie.setHttpOnly(true);

    c->res()->addCookie(sessionCookie);
}

void SessionPrivate::deleteSession(Context *c, const QString &reason)
{
    qCDebug(C_SESSION) << "Deleting session" << reason;

    c->setProperty(SESSION_VALUES, QVariant());
    c->setProperty(SESSION_SAVE, true);
    // to prevent getSessionId from returning it
    c->setProperty(SESSION_DELETED_ID, true);

    const QString &sessionFile = SessionPrivate::filePath(c->property(SESSION_ID).toString());
    bool ret = QFile::remove(sessionFile);
    if (!ret) {
        QSettings settings(sessionFile, QSettings::IniFormat);
        settings.clear();
    }
}

QVariant SessionPrivate::loadSession(Context *c, bool createSessionId)
{
    const QVariant &property = c->property(SESSION_VALUES);
    if (!property.isNull()) {
        return property.toHash();
    }

    Session *session = c->plugin<Session*>();
    if (!session) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return QVariant();
    }

    QString sessionid = getSessionId(c, session->d_ptr->sessionName);
    if (SessionPrivate::validateSessionId(sessionid)) {
        quint64 expires;
        const QVariantHash &sessionHash = session->retrieveSession(sessionid, expires);
        if (expires < QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000) {
            deleteSession(c, "session expired");
            session->persistSession(sessionid, QVariant(), 0);
            return QVariantHash();
        }

        expires = qMax(expires, (QDateTime::currentDateTimeUtc().toMSecsSinceEpoch() / 1000) + session->d_ptr->sessionExpires);
        c->setProperty(SESSION_EXPIRES, expires);
        c->setProperty(SESSION_VALUES, sessionHash);
        return sessionHash;
    } else if (createSessionId) {
        SessionPrivate::createSessionId(c, session->d_ptr->sessionExpires);
    }

    return QVariantHash();
}

bool SessionPrivate::validateSessionId(const QString &id)
{
    int i = 0;
    int size = id.size();
    while (i < size) {
        QChar c = id[i];
        if ((c >= 'a' && c <= 'f') || (c >= '0' && c <= '9')) {
            ++i;
            continue;
        }
        return false;
    }

    return size;
}
