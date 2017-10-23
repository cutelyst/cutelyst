/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include "sessionstorefile.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Response>
#include <Cutelyst/Engine>

#include <QUuid>
#include <QHostAddress>
#include <QLoggingCategory>
#include <QCoreApplication>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION, "cutelyst.plugin.session")

#define SESSION_VALUES "__session_values"
#define SESSION_EXPIRES "__session_expires"
#define SESSION_TRIED_LOADING_EXPIRES "__session_tried_loading_expires"
#define SESSION_EXTENDED_EXPIRES "__session_extended_expires"
#define SESSION_UPDATED "__session_updated"
#define SESSION_ID "__session_id"
#define SESSION_TRIED_LOADING_ID "__session_tried_loading_id"
#define SESSION_DELETED_ID "__session_deleted_id"
#define SESSION_DELETE_REASON "__session_delete_reason"

static thread_local Session *m_instance = nullptr;

Session::Session(Cutelyst::Application *parent) : Plugin(parent)
  , d_ptr(new SessionPrivate(this))
{

}

Cutelyst::Session::~Session()
{
    delete d_ptr;
}

bool Session::setup(Application *app)
{
    Q_D(Session);
    d->sessionName = QCoreApplication::applicationName() + QLatin1String("_session");

    const QVariantMap config = app->engine()->config(QLatin1String("Cutelyst_Session_Plugin"));
    d->sessionExpires = config.value(QLatin1String("expires"), 7200).toULongLong();
    d->expiryThreshold = config.value(QLatin1String("expiry_threshold"), 0).toULongLong();
    d->verifyAddress = config.value(QLatin1String("verify_address"), false).toBool();
    d->verifyUserAgent = config.value(QLatin1String("verify_user_agent"), false).toBool();

    connect(app, &Application::afterDispatch, this, &SessionPrivate::_q_saveSession);
    connect(app, &Application::postForked, this, &SessionPrivate::_q_postFork);

    if (!d->store) {
        d->store = new SessionStoreFile(this);
    }

    return true;
}

void Session::setStorage(SessionStore *store)
{
    Q_D(Session);
    if (d->store) {
        qFatal("Session Storage is alread defined");
    }
    store->setParent(this);
    d->store = store;
}

SessionStore *Session::storage() const
{
    Q_D(const Session);
    return d->store;
}

QString Session::id(Cutelyst::Context *c)
{
    QString ret;
    const QVariant sid = c->property(SESSION_ID);
    if (sid.isNull()) {
        if (Q_UNLIKELY(!m_instance)) {
            qCCritical(C_SESSION) << "Session plugin not registered";
            return ret;
        }

        ret = SessionPrivate::loadSessionId(c, m_instance->d_ptr->sessionName);
    } else {
        ret = sid.toString();
    }

    return ret;
}

quint64 Session::expires(Context *c)
{
    QVariant expires = c->property(SESSION_EXTENDED_EXPIRES);
    if (!expires.isNull()) {
        return expires.toULongLong();
    }

    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return 0;
    }

    expires = SessionPrivate::loadSessionExpires(m_instance, c, id(c));
    if (!expires.isNull()) {
        return SessionPrivate::extendSessionExpires(m_instance, c, expires.toULongLong());
    }

    return 0;
}

void Session::changeExpires(Context *c, quint64 expires)
{
    const QString sid = Session::id(c);
    const quint64 timeExp = (QDateTime::currentMSecsSinceEpoch() / 1000) + expires;

    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return;
    }

    m_instance->d_ptr->store->storeSessionData(c, sid, QStringLiteral("expires"), timeExp);
}

void Session::deleteSession(Context *c, const QString &reason)
{
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return;
    }
    SessionPrivate::deleteSession(m_instance, c, reason);
}

QString Session::deleteReason(Context *c)
{
    return c->property(SESSION_DELETE_REASON).toString();
}

QVariant Session::value(Cutelyst::Context *c, const QString &key, const QVariant &defaultValue)
{
    QVariant ret = defaultValue;
    QVariant session = c->property(SESSION_VALUES);
    if (session.isNull()) {
        session = SessionPrivate::loadSession(c);
    }

    if (!session.isNull()) {
        ret = session.toHash().value(key, defaultValue);
    }

    return ret;
}

void Session::setValue(Cutelyst::Context *c, const QString &key, const QVariant &value)
{
    QVariant session = c->property(SESSION_VALUES);
    if (session.isNull()) {
        session = SessionPrivate::loadSession(c);
        if (session.isNull()) {
            if (Q_UNLIKELY(!m_instance)) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return;
            }

            SessionPrivate::createSessionIdIfNeeded(m_instance, c, m_instance->d_ptr->sessionExpires);
            session = SessionPrivate::initializeSessionData(m_instance, c);
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
            if (Q_UNLIKELY(!m_instance)) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return;
            }

            SessionPrivate::createSessionIdIfNeeded(m_instance, c, m_instance->d_ptr->sessionExpires);
            session = SessionPrivate::initializeSessionData(m_instance, c);
        }
    }

    QVariantHash data = session.toHash();
    data.remove(key);

    c->setProperty(SESSION_VALUES, data);
    c->setProperty(SESSION_UPDATED, true);
}

void Session::deleteValues(Context *c, const QStringList &keys)
{
    QVariant session = c->property(SESSION_VALUES);
    if (session.isNull()) {
        session = SessionPrivate::loadSession(c);
        if (session.isNull()) {
            if (Q_UNLIKELY(!m_instance)) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return;
            }

            SessionPrivate::createSessionIdIfNeeded(m_instance, c, m_instance->d_ptr->sessionExpires);
            session = SessionPrivate::initializeSessionData(m_instance, c);
        }
    }

    QVariantHash data = session.toHash();
    for (const QString &key : keys) {
        data.remove(key);
    }

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
    QString ret;
    if (!c->property(SESSION_TRIED_LOADING_ID).isNull()) {
        return ret;
    }
    c->setProperty(SESSION_TRIED_LOADING_ID, true);

    const QString sid = getSessionId(c, sessionName);
    if (!sid.isEmpty() && !validateSessionId(sid)) {
        qCCritical(C_SESSION) << "Tried to set invalid session ID" << sid;
        return ret;
    }

    ret = sid;
    c->setProperty(SESSION_ID, sid);
    return ret;
}

QString SessionPrivate::getSessionId(Context *c, const QString &sessionName)
{
    QString ret;
    bool deleted = !c->property(SESSION_DELETED_ID).isNull();

    if (!deleted) {
        const QVariant property = c->property(SESSION_ID);
        if (!property.isNull()) {
            ret = property.toString();
            return ret;
        }

        const QString cookie = c->request()->cookie(sessionName);
        if (!cookie.isEmpty()) {
            qCDebug(C_SESSION) << "Found sessionid" << cookie << "in cookie";
            ret = cookie;
        }
    }

    return ret;
}

QString SessionPrivate::createSessionIdIfNeeded(Session *session, Context *c, quint64 expires)
{
    QString ret;
    const QVariant sid = c->property(SESSION_ID);
    if (!sid.isNull()) {
        ret = sid.toString();
    } else {
        ret = createSessionId(session, c, expires);
    }
    return ret;
}

QString SessionPrivate::createSessionId(Session *session, Context *c, quint64 expires)
{
    const QString sid = generateSessionId();

    qCDebug(C_SESSION) << "Created session" << sid;

    c->setProperty(SESSION_ID, sid);
    resetSessionExpires(session, c, sid);
    setSessionId(session, c, sid);

    return sid;
}

void SessionPrivate::_q_saveSession(Context *c)
{
    // fix cookie before we send headers
    saveSessionExpires(c);

    // Force extension of session_expires before finalizing headers, so a pos
    // up to date. First call to session_expires will extend the expiry, methods
    // just return the previously extended value.
    Session::expires(c);

    // Persist data
    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return;
    }
    saveSessionExpires(c);

    if (!c->property(SESSION_UPDATED).toBool()) {
        return;
    }
    SessionStore *store = m_instance->d_ptr->store;
    QVariantHash sessionData = c->property(SESSION_VALUES).toHash();
    sessionData.insert(QStringLiteral("__updated"), QDateTime::currentMSecsSinceEpoch() / 1000);

    const QString sid = c->property(SESSION_ID).toString();
    store->storeSessionData(c, sid,  QStringLiteral("session"), sessionData);
}

void SessionPrivate::_q_postFork(Application *app)
{
    m_instance = app->plugin<Session *>();
}

void SessionPrivate::deleteSession(Session *session, Context *c, const QString &reason)
{
    qCDebug(C_SESSION) << "Deleting session" << reason;

    const QVariant sidVar = c->property(SESSION_ID).toString();
    if (!sidVar.isNull()) {
        const QString sid = sidVar.toString();
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("session"));
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("expires"));
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("flash"));

        deleteSessionId(session, c, sid);
    }

    // Reset the values in Context object
    c->setProperty(SESSION_VALUES, QVariant());
    c->setProperty(SESSION_ID, QVariant());
    c->setProperty(SESSION_EXPIRES, QVariant());

    c->setProperty(SESSION_DELETE_REASON, reason);
}

void SessionPrivate::deleteSessionId(Session *session, Context *c, const QString &sid)
{
    c->setProperty(SESSION_DELETED_ID, true); // to prevent get_session_id from returning it

    updateSessionCookie(c, makeSessionCookie(session, c, sid, QDateTime::currentDateTimeUtc()));
}

QVariant SessionPrivate::loadSession(Context *c)
{
    QVariant ret;
    const QVariant property = c->property(SESSION_VALUES);
    if (!property.isNull()) {
        ret = property.toHash();
        return ret;
    }

    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return ret;
    }

    const QString sid = Session::id(c);
    if (!loadSessionExpires(m_instance, c, sid).isNull()) {
        if (SessionPrivate::validateSessionId(sid)) {

            const QVariantHash sessionData = m_instance->d_ptr->store->getSessionData(c, sid, QStringLiteral("session")).toHash();
            c->setProperty(SESSION_VALUES, sessionData);

            if (m_instance->d_ptr->verifyAddress &&
                    sessionData.contains(QStringLiteral("__address")) &&
                    sessionData.value(QStringLiteral("__address")).toString() != c->request()->address().toString()) {
                qCWarning(C_SESSION) << "Deleting session" << sid << "due to address mismatch:"
                                     << sessionData.value(QStringLiteral("__address")).toString()
                                     << "!="
                                     << c->request()->address().toString();
                deleteSession(m_instance, c, QStringLiteral("address mismatch"));
                return ret;
            }

            if (m_instance->d_ptr->verifyUserAgent &&
                    sessionData.contains(QStringLiteral("__user_agent")) &&
                    sessionData.value(QStringLiteral("__user_agent")).toString() != c->request()->userAgent()) {
                qCWarning(C_SESSION) << "Deleting session" << sid << "due to user agent mismatch:"
                                     << sessionData.value(QStringLiteral("__user_agent")).toString()
                                     << "!="
                                     << c->request()->userAgent();
                deleteSession(m_instance, c, QStringLiteral("user agent mismatch"));
                return ret;
            }

            qCDebug(C_SESSION) << "Restored session" << sid;

            ret = sessionData;
        }
    }

    return ret;
}

bool SessionPrivate::validateSessionId(const QString &id)
{
    auto it = id.constBegin();
    auto end = id.constEnd();
    while (it != end) {
        QChar c = *it;
        if ((c >= QLatin1Char('a') && c <= QLatin1Char('f')) || (c >= QLatin1Char('0') && c <= QLatin1Char('9'))) {
            ++it;
            continue;
        }
        return false;
    }

    return id.size();
}

quint64 SessionPrivate::extendSessionExpires(Session *session, Context *c, quint64 expires)
{
    const quint64 threshold = session->d_ptr->expiryThreshold;

    const QString sid = Session::id(c);
    if (!sid.isEmpty()) {
        const quint64 current = getStoredSessionExpires(session, c, sid);
        const quint64 cutoff = current - threshold;
        const quint64 time = QDateTime::currentMSecsSinceEpoch() / 1000;

        if (!threshold || cutoff <= time || c->property(SESSION_UPDATED).toBool()) {
            quint64 updated = calculateInitialSessionExpires(session, c, sid);
            c->setProperty(SESSION_EXTENDED_EXPIRES, updated);
            extendSessionId(session, c, sid, updated);

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
    const QVariant expires = session->d_ptr->store->getSessionData(c, sessionid, QStringLiteral("expires"), 0);
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

void SessionPrivate::saveSessionExpires(Context *c)
{
    const QVariant expires = c->property(SESSION_EXPIRES);
    if (!expires.isNull()) {
        const QString sid = Session::id(c);
        if (!sid.isEmpty()) {
            if (Q_UNLIKELY(!m_instance)) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return;
            }

            const quint64 current = getStoredSessionExpires(m_instance, c, sid);
            const quint64 extended = Session::expires(c);
            if (extended > current) {
                m_instance->d_ptr->store->storeSessionData(c, sid, QStringLiteral("expires"), extended);
            }
        }
    }
}

QVariant SessionPrivate::loadSessionExpires(Session *session, Context *c, const QString &sessionId)
{
    QVariant ret;
    if (c->property(SESSION_TRIED_LOADING_EXPIRES).toBool()) {
        ret = c->property(SESSION_EXPIRES);
        return ret;
    }
    c->setProperty(SESSION_TRIED_LOADING_EXPIRES, true);

    if (!sessionId.isEmpty()) {
        const quint64 expires = getStoredSessionExpires(session, c, sessionId);

        if (expires >= static_cast<quint64>(QDateTime::currentMSecsSinceEpoch() / 1000)) {
            c->setProperty(SESSION_EXPIRES, expires);
            ret = expires;
        } else {
            deleteSession(session, c, QStringLiteral("session expired"));
            ret = 0;
        }
    }
    return ret;
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

void SessionPrivate::updateSessionCookie(Context *c, const QNetworkCookie &updated)
{
    c->response()->setCookie(updated);
}

QNetworkCookie SessionPrivate::makeSessionCookie(Session *session, Context *c, const QString &sid, const QDateTime &expires)
{
    QNetworkCookie cookie(session->d_ptr->sessionName.toLatin1(), sid.toLatin1());
    cookie.setPath(QStringLiteral("/"));
    cookie.setExpirationDate(expires);
    cookie.setHttpOnly(session->d_ptr->cookieHttpOnly);
    cookie.setSecure(session->d_ptr->cookieSecure);

    return cookie;
}

void SessionPrivate::extendSessionId(Session *session, Context *c, const QString &sid, quint64 expires)
{
    updateSessionCookie(c, makeSessionCookie(session, c, sid, QDateTime::fromMSecsSinceEpoch(expires * 1000)));
}

void SessionPrivate::setSessionId(Session *session, Context *c, const QString &sid)
{
    updateSessionCookie(c, makeSessionCookie(session, c, sid,
                                             QDateTime::fromMSecsSinceEpoch(initialSessionExpires(session, c) * 1000)));
}

SessionStore::SessionStore(QObject *parent) : QObject(parent)
{

}

#include "moc_session.cpp"
