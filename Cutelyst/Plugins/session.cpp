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

#include "sessionstorefile.h"

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
#define SESSION_TRIED_LOADING_EXPIRES "__session_tried_loading_expires"
#define SESSION_EXTENDED_EXPIRES "__session_extended_expires"
#define SESSION_UPDATED "__session_updated"
#define SESSION_ID "__session_id"
#define SESSION_DELETED_ID "__session_deleted_id"
#define SESSION_DELETE_REASON "__session_delete_reason"

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

    const QVariantHash &config = app->config("Session_Plugin").toHash();
    d->sessionExpires = config.value("expires", 7200).toULongLong();
    d->expiryThreshold = config.value("expiry_threshold", 0).toULongLong();
    d->verifyAddress = config.value("verify_address", false).toBool();
    d->verifyUserAgent = config.value("verify_user_agent", false).toBool();

    connect(app, &Application::afterDispatch, d, &SessionPrivate::saveSession);

    if (!d->store) {
        d->store = new SessionStoreFile;
    }

    return true;
}

void Session::setStorage(SessionStore *store)
{
    Q_D(Session);
    if (d->store) {
        qFatal("Session Storage is alread defined");
    }
    d->store = store;
}

QString Session::id(Cutelyst::Context *c)
{
    const QVariant &sid = c->property(SESSION_ID);
    if (sid.isNull()) {
        Session *session = c->plugin<Session*>();
        if (!session) {
            qCCritical(C_SESSION) << "Session plugin not registered";
            return QString();
        }

        return SessionPrivate::loadSessionId(c, session->d_ptr->sessionName);
    }

    return sid.toString();
}

quint64 Session::expires(Context *c)
{
    QVariant expires = c->property(SESSION_EXTENDED_EXPIRES);
    if (!expires.isNull()) {
        return expires.toULongLong();
    }

    Session *session = c->plugin<Session*>();
    if (!session) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return 0;
    }

    expires = SessionPrivate::loadSessionExpires(session, c, id(c));
    if (!expires.isNull()) {
        return SessionPrivate::extendSessionExpires(session, c, expires.toULongLong());
    }

    return 0;
}

void Session::changeExpires(Context *c, quint64 expires)
{
    const QString &sid = Session::id(c);
    const quint64 timeExp = (QDateTime::currentMSecsSinceEpoch() / 1000) + expires;

    Session *session = c->plugin<Session*>();
    if (!session) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return;
    }

    session->d_ptr->store->storeSessionData(c, sid, QStringLiteral("expires"), timeExp);
}

void Session::deleteSession(Context *c, const QString &reason)
{
    Session *session = c->plugin<Session*>();
    if (!session) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return;
    }
    SessionPrivate::deleteSession(session, c, reason);
}

QString Session::deleteReason(Context *c)
{
    return c->property(SESSION_DELETE_REASON).toString();
}

QVariant Session::value(Cutelyst::Context *c, const QString &key, const QVariant &defaultValue)
{
    QVariant session = c->property(SESSION_VALUES);
    if (session.isNull()) {
        session = SessionPrivate::loadSession(c);
        if (session.isNull()) {
            Session *plugin = c->plugin<Session*>();
            if (!plugin) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return QVariant();
            }

            SessionPrivate::createSessionIdIfNeeded(plugin, c, plugin->d_ptr->sessionExpires);
            session = SessionPrivate::initializeSessionData(plugin, c);
            c->setProperty(SESSION_VALUES, session);
        }
    }

    const QVariantHash &data = session.toHash();
    return data.value(key, defaultValue);
}

void Session::setValue(Cutelyst::Context *c, const QString &key, const QVariant &value)
{
    QVariant session = c->property(SESSION_VALUES);
    if (session.isNull()) {
        session = SessionPrivate::loadSession(c);
        if (session.isNull()) {
            Session *plugin = c->plugin<Session*>();
            if (!plugin) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return;
            }

            SessionPrivate::createSessionIdIfNeeded(plugin, c, plugin->d_ptr->sessionExpires);
            session = SessionPrivate::initializeSessionData(plugin, c);
        }
    }

    QVariantHash data = session.toHash();
    data.insert(key, value);

    c->setProperty(SESSION_VALUES, data);
    c->setProperty(SESSION_UPDATED, true);
}

void Session::deleteValue(Context *c, const QString &key)
{
    QVariant session = c->property(SESSION_VALUES);
    if (session.isNull()) {
        session = SessionPrivate::loadSession(c);
        if (session.isNull()) {
            Session *plugin = c->plugin<Session*>();
            if (!plugin) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return;
            }

            SessionPrivate::createSessionIdIfNeeded(plugin, c, plugin->d_ptr->sessionExpires);
            session = SessionPrivate::initializeSessionData(plugin, c);
        }
    }

    QVariantHash data = session.toHash();
    data.remove(key);

    c->setProperty(SESSION_VALUES, data);
    c->setProperty(SESSION_UPDATED, true);
}

bool Session::isValid(Cutelyst::Context *c)
{
    return !SessionPrivate::loadSession(c).isNull();
}

QString SessionPrivate::generateSessionId()
{
    return QString::fromLatin1(QUuid::createUuid().toRfc4122().toHex());
}

QString SessionPrivate::loadSessionId(Context *c, const QString &sessionName)
{
    const QString &sid = getSessionId(c, sessionName);
    if (!sid.isNull() && !validateSessionId(sid)) {
        qCCritical(C_SESSION) << "Tried to set invalid session ID" << sid;
        c->setProperty(SESSION_ID, QString());
        return QString();
    }

    c->setProperty(SESSION_ID, sid);
    return sid;
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
    }

    return sessionId;
}

QString SessionPrivate::createSessionIdIfNeeded(Session *session, Context *c, quint64 expires)
{
    const QVariant &sid = c->property(SESSION_ID);
    if (!sid.isNull()) {
        return sid.toString();
    }

    return createSessionId(session, c, expires);
}

QString SessionPrivate::createSessionId(Session *session, Context *c, quint64 expires)
{
    QString sessionId = generateSessionId();
    qCDebug(C_SESSION) << "Created session" << sessionId;
    c->setProperty(SESSION_ID, sessionId);
    resetSessionExpires(session, c, sessionId);
    c->setProperty(SESSION_DELETED_ID, QVariant());
    return sessionId;
}

void SessionPrivate::saveSession(Context *c)
{
    if (!c->property(SESSION_UPDATED).toBool()) {
        return;
    }

    Session *session = c->plugin<Session*>();
    if (!session) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return;
    }

    const QString &_sessionName = session->d_ptr->sessionName;
    const QString &sid = c->property(SESSION_ID).toString();

    bool deleted = !c->property(SESSION_DELETED_ID).isNull();
    QDateTime expiresDt;
    if (deleted) {
        expiresDt = QDateTime::currentDateTimeUtc();
    } else {
        expiresDt = saveSessionExpires(c);

        SessionStore *store = session->d_ptr->store;
        QVariantHash sessionData = c->property(SESSION_VALUES).toHash();
        sessionData.insert(QStringLiteral("__updated"), QDateTime::currentMSecsSinceEpoch() / 1000);

        store->storeSessionData(c, sid,  QStringLiteral("session"), sessionData);
    }

    QNetworkCookie sessionCookie(_sessionName.toLatin1(), sid.toLatin1());
    sessionCookie.setPath(QStringLiteral("/"));
    sessionCookie.setExpirationDate(expiresDt);
    sessionCookie.setHttpOnly(true);

    c->res()->addCookie(sessionCookie);
}

void SessionPrivate::deleteSession(Session *session, Context *c, const QString &reason)
{
    qCDebug(C_SESSION) << "Deleting session" << reason;

    const QVariant &sidVar = c->property(SESSION_ID).toString();
    if (!sidVar.isNull()) {
        const QString &sid = sidVar.toString();
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("session"));
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("expires"));
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("flash"));
    }

    c->setProperty(SESSION_DELETE_REASON, reason);
    c->setProperty(SESSION_VALUES, QVariant());
    // So an expired coockie is sent
    c->setProperty(SESSION_UPDATED, true);
    // to prevent getSessionId from returning it
    c->setProperty(SESSION_DELETED_ID, true);
    c->setProperty(SESSION_ID, QVariant());
    c->setProperty(SESSION_EXPIRES, QVariant());
}

QVariant SessionPrivate::loadSession(Context *c)
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

    const QString &sid = Session::id(c);
    if (!loadSessionExpires(session, c, sid).isNull()) {
        if (SessionPrivate::validateSessionId(sid)) {

            const QVariantHash &sessionData = session->d_ptr->store->getSessionData(c, sid, QStringLiteral("session")).toHash();
            c->setProperty(SESSION_VALUES, sessionData);

            if (session->d_ptr->verifyAddress &&
                    sessionData.contains(QStringLiteral("__address")) &&
                    sessionData.value(QStringLiteral("__address")).toString() != c->request()->address().toString()) {
                qCWarning(C_SESSION) << "Deleting session" << sid << "due to address mismatch:"
                                     << sessionData.value(QStringLiteral("__address")).toString()
                                     << "!="
                                     << c->request()->address().toString();
                deleteSession(session, c, QStringLiteral("address mismatch"));
                return QVariant();
            }

            if (session->d_ptr->verifyUserAgent &&
                    sessionData.contains(QStringLiteral("__user_agent")) &&
                    sessionData.value(QStringLiteral("__user_agent")).toString() != c->request()->userAgent()) {
                qCWarning(C_SESSION) << "Deleting session" << sid << "due to user agent mismatch:"
                                     << sessionData.value(QStringLiteral("__user_agent")).toString()
                                     << "!="
                                     << c->request()->userAgent();
                deleteSession(session, c, QStringLiteral("user agent mismatch"));
                return QVariant();
            }

            qCDebug(C_SESSION) << "Restored session" << sid;

            return sessionData;
        }
    }

    return QVariant();
}

bool SessionPrivate::validateSessionId(const QString &id)
{
    int i = 0;
    const int size = id.size();
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

quint64 SessionPrivate::extendSessionExpires(Session *session, Context *c, quint64 expires)
{
    const quint64 threshold = session->d_ptr->expiryThreshold;

    const QString &sid = Session::id(c);
    if (!sid.isEmpty()) {
        const quint64 current = getStoredSessionExpires(session, c, sid);
        const quint64 cutoff = current - threshold;
        const quint64 time = QDateTime::currentMSecsSinceEpoch() / 1000;

        if (!threshold || cutoff <= time || c->property(SESSION_UPDATED).toBool()) {
            quint64 updated = calculateInitialSessionExpires(session, c, sid);
            c->setProperty(SESSION_EXTENDED_EXPIRES, updated);

            return updated;
        } else {
            return current;
        }
    } else {
        return expires;
    }
}

quint64 SessionPrivate::getStoredSessionExpires(Session *session, Context *c, const QString &sessionid)
{
    const QVariant &expires = session->d_ptr->store->getSessionData(c, sessionid, QStringLiteral("expires"), 0);
    return expires.toULongLong();
}

QVariant SessionPrivate::initializeSessionData(Session *session, Context *c)
{
    QVariantHash ret;
    const quint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;
    ret.insert(QStringLiteral("__created"), now);
    ret.insert(QStringLiteral("__updated"), now);

    if (session->d_ptr->verifyAddress) {
        ret.insert(QStringLiteral("__address"), c->request()->address().toString());
    }

    if (session->d_ptr->verifyUserAgent) {
        ret.insert(QStringLiteral("__user_agent"), c->request()->userAgent());
    }

    return ret;
}

QDateTime SessionPrivate::saveSessionExpires(Context *c)
{
    const QVariant &expires = c->property(SESSION_EXPIRES);
    if (expires.isNull()) {
        return QDateTime::currentDateTimeUtc();
    }

    const QString &sid = Session::id(c);
    if (!sid.isEmpty()) {
        Session *session = c->plugin<Session*>();
        if (!session) {
            qCCritical(C_SESSION) << "Session plugin not registered";
            return QDateTime::currentDateTimeUtc();
        }

        const quint64 current = getStoredSessionExpires(session, c, sid);
        const quint64 extended = Session::expires(c);
        if (extended > current) {
            session->d_ptr->store->storeSessionData(c, sid, QStringLiteral("expires"), extended);

            return QDateTime::fromMSecsSinceEpoch(extended * 1000);
        }
    }

    return QDateTime::currentDateTimeUtc();
}

QVariant SessionPrivate::loadSessionExpires(Session *session, Context *c, const QString &sessionId)
{
    if (c->property(SESSION_TRIED_LOADING_EXPIRES).toBool()) {
        return c->property(SESSION_EXPIRES);
    }
    c->setProperty(SESSION_TRIED_LOADING_EXPIRES, true);

    if (!sessionId.isEmpty()) {
        const quint64 expires = getStoredSessionExpires(session, c, sessionId);

        if (expires >= QDateTime::currentMSecsSinceEpoch() / 1000) {
            c->setProperty(SESSION_EXPIRES, expires);
            return expires;
        } else {
            deleteSession(session, c, QStringLiteral("session expired"));
            return 0;
        }
    }
    return QVariant();
}

quint64 SessionPrivate::initialSessionExpires(Session *session, Context *c)
{
    const quint64 expires = session->d_ptr->sessionExpires;
    return (QDateTime::currentMSecsSinceEpoch() / 1000) + expires;
}

quint64 SessionPrivate::calculateInitialSessionExpires(Session *session, Context *c, const QString &sessionId)
{
    const quint64 stored = getStoredSessionExpires(session, c, sessionId);
    const quint64 initial = initialSessionExpires(session, c);
    return qMax(initial , stored);
}

quint64 SessionPrivate::resetSessionExpires(Session *session, Context *c, const QString &sessionId)
{
    const quint64 exp = calculateInitialSessionExpires(session, c, sessionId);

    c->setProperty(SESSION_EXPIRES, exp);

    // since we're setting _session_expires directly, make loadSessionExpires
    // actually use that value.
    c->setProperty(SESSION_TRIED_LOADING_EXPIRES, true);
    c->setProperty(SESSION_EXTENDED_EXPIRES, exp);

    return exp;
}

SessionStore::SessionStore(QObject *parent) : QObject(parent)
{

}
