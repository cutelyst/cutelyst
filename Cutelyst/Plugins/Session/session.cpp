/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "session_p.h"
#include "sessionstorefile.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Engine>
#include <Cutelyst/Response>

#include <QCoreApplication>
#include <QHostAddress>
#include <QLoggingCategory>
#include <QUuid>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION, "cutelyst.plugin.session", QtWarningMsg)

#define SESSION_VALUES QStringLiteral("_c_session_values")
#define SESSION_EXPIRES QStringLiteral("_c_session_expires")
#define SESSION_TRIED_LOADING_EXPIRES QStringLiteral("_c_session_tried_loading_expires")
#define SESSION_EXTENDED_EXPIRES QStringLiteral("_c_session_extended_expires")
#define SESSION_UPDATED QStringLiteral("_c_session_updated")
#define SESSION_ID QStringLiteral("_c_session_id")
#define SESSION_TRIED_LOADING_ID QStringLiteral("_c_session_tried_loading_id")
#define SESSION_DELETED_ID QStringLiteral("_c_session_deleted_id")
#define SESSION_DELETE_REASON QStringLiteral("_c_session_delete_reason")

static thread_local Session *m_instance = nullptr;

Session::Session(Cutelyst::Application *parent)
    : Plugin(parent)
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
    d->sessionExpires        = config.value(QLatin1String("expires"), 7200).toLongLong();
    d->expiryThreshold       = config.value(QLatin1String("expiry_threshold"), 0).toLongLong();
    d->verifyAddress         = config.value(QLatin1String("verify_address"), false).toBool();
    d->verifyUserAgent       = config.value(QLatin1String("verify_user_agent"), false).toBool();
    d->cookieHttpOnly        = config.value(QLatin1String("cookie_http_only"), true).toBool();
    d->cookieSecure          = config.value(QLatin1String("cookie_secure"), false).toBool();
    const QString _sameSite  = config.value(QLatin1String("cookie_same_site"), QStringLiteral("strict")).toString();
#if QT_VERSION >= QT_VERSION_CHECK(6, 1, 0)
    if (_sameSite.compare(u"default", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Default;
    } else if (_sameSite.compare(u"none", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::None;
    } else if (_sameSite.compare(u"lax", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Lax;
    } else {
        d->cookieSameSite = QNetworkCookie::SameSite::Strict;
    }
#else
    if (_sameSite.compare(u"default", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = Cookie::SameSite::Default;
    } else if (_sameSite.compare(u"none", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = Cookie::SameSite::None;
    } else if (_sameSite.compare(u"lax", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = Cookie::SameSite::Lax;
    } else {
        d->cookieSameSite = Cookie::SameSite::Strict;
    }
#endif

    connect(app, &Application::afterDispatch, this, &SessionPrivate::_q_saveSession);
    connect(app, &Application::postForked, this, [=] {
        m_instance = this;
    });

    if (!d->store) {
        d->store = new SessionStoreFile(this);
    }

    return true;
}

void Session::setStorage(SessionStore *store)
{
    Q_D(Session);
    Q_ASSERT_X(d->store, "Cutelyst::Session::setStorage", "Session Storage is alread defined");
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
    const QVariant sid = c->stash(SESSION_ID);
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
    QVariant expires = c->stash(SESSION_EXTENDED_EXPIRES);
    if (!expires.isNull()) {
        return expires.toULongLong();
    }

    if (Q_UNLIKELY(!m_instance)) {
        qCCritical(C_SESSION) << "Session plugin not registered";
        return 0;
    }

    expires = SessionPrivate::loadSessionExpires(m_instance, c, id(c));
    if (!expires.isNull()) {
        return quint64(SessionPrivate::extendSessionExpires(m_instance, c, expires.toLongLong()));
    }

    return 0;
}

void Session::changeExpires(Context *c, quint64 expires)
{
    const QString sid    = Session::id(c);
    const qint64 timeExp = QDateTime::currentMSecsSinceEpoch() / 1000 + qint64(expires);

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
    return c->stash(SESSION_DELETE_REASON).toString();
}

QVariant Session::value(Cutelyst::Context *c, const QString &key, const QVariant &defaultValue)
{
    QVariant ret     = defaultValue;
    QVariant session = c->stash(SESSION_VALUES);
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
    QVariant session = c->stash(SESSION_VALUES);
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

    c->setStash(SESSION_VALUES, data);
    c->setStash(SESSION_UPDATED, true);
}

void Session::deleteValue(Context *c, const QString &key)
{
    QVariant session = c->stash(SESSION_VALUES);
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

    c->setStash(SESSION_VALUES, data);
    c->setStash(SESSION_UPDATED, true);
}

void Session::deleteValues(Context *c, const QStringList &keys)
{
    QVariant session = c->stash(SESSION_VALUES);
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

    c->setStash(SESSION_VALUES, data);
    c->setStash(SESSION_UPDATED, true);
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
    if (!c->stash(SESSION_TRIED_LOADING_ID).isNull()) {
        return ret;
    }
    c->setStash(SESSION_TRIED_LOADING_ID, true);

    const QString sid = getSessionId(c, sessionName);
    if (!sid.isEmpty()) {
        if (!validateSessionId(sid)) {
            qCCritical(C_SESSION) << "Tried to set invalid session ID" << sid;
            return ret;
        }
        ret = sid;
        c->setStash(SESSION_ID, sid);
    }

    return ret;
}

QString SessionPrivate::getSessionId(Context *c, const QString &sessionName)
{
    QString ret;
    bool deleted = !c->stash(SESSION_DELETED_ID).isNull();

    if (!deleted) {
        const QVariant property = c->stash(SESSION_ID);
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

QString SessionPrivate::createSessionIdIfNeeded(Session *session, Context *c, qint64 expires)
{
    QString ret;
    const QVariant sid = c->stash(SESSION_ID);
    if (!sid.isNull()) {
        ret = sid.toString();
    } else {
        ret = createSessionId(session, c, expires);
    }
    return ret;
}

QString SessionPrivate::createSessionId(Session *session, Context *c, qint64 expires)
{
    Q_UNUSED(expires)
    const QString sid = generateSessionId();

    qCDebug(C_SESSION) << "Created session" << sid;

    c->setStash(SESSION_ID, sid);
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

    if (!c->stash(SESSION_UPDATED).toBool()) {
        return;
    }
    SessionStore *store      = m_instance->d_ptr->store;
    QVariantHash sessionData = c->stash(SESSION_VALUES).toHash();
    sessionData.insert(QStringLiteral("__updated"), QDateTime::currentMSecsSinceEpoch() / 1000);

    const QString sid = c->stash(SESSION_ID).toString();
    store->storeSessionData(c, sid, QStringLiteral("session"), sessionData);
}

void SessionPrivate::deleteSession(Session *session, Context *c, const QString &reason)
{
    qCDebug(C_SESSION) << "Deleting session" << reason;

    const QVariant sidVar = c->stash(SESSION_ID).toString();
    if (!sidVar.isNull()) {
        const QString sid = sidVar.toString();
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("session"));
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("expires"));
        session->d_ptr->store->deleteSessionData(c, sid, QStringLiteral("flash"));

        deleteSessionId(session, c, sid);
    }

    // Reset the values in Context object
    c->setStash(SESSION_VALUES, QVariant());
    c->setStash(SESSION_ID, QVariant());
    c->setStash(SESSION_EXPIRES, QVariant());

    c->setStash(SESSION_DELETE_REASON, reason);
}

void SessionPrivate::deleteSessionId(Session *session, Context *c, const QString &sid)
{
    c->setStash(SESSION_DELETED_ID, true); // to prevent get_session_id from returning it

#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    updateSessionCookie(c, makeSessionCookie(session, c, sid, QDateTime::currentDateTimeUtc()));
#else
    updateSessionCuteCookie(c, makeSessionCuteCookie(session, c, sid, QDateTime::currentDateTimeUtc()));
#endif
}

QVariant SessionPrivate::loadSession(Context *c)
{
    QVariant ret;
    const QVariant property = c->stash(SESSION_VALUES);
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
            c->setStash(SESSION_VALUES, sessionData);

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
    auto it  = id.constBegin();
    auto end = id.constEnd();
    while (it != end) {
        QChar c = *it;
        if ((c >= u'a' && c <= u'f') || (c >= u'0' && c <= u'9')) {
            ++it;
            continue;
        }
        return false;
    }

    return id.size();
}

qint64 SessionPrivate::extendSessionExpires(Session *session, Context *c, qint64 expires)
{
    const qint64 threshold = qint64(session->d_ptr->expiryThreshold);

    const QString sid = Session::id(c);
    if (!sid.isEmpty()) {
        const qint64 current = getStoredSessionExpires(session, c, sid);
        const qint64 cutoff  = current - threshold;
        const qint64 time    = QDateTime::currentMSecsSinceEpoch() / 1000;

        if (!threshold || cutoff <= time || c->stash(SESSION_UPDATED).toBool()) {
            qint64 updated = calculateInitialSessionExpires(session, c, sid);
            c->setStash(SESSION_EXTENDED_EXPIRES, updated);
            extendSessionId(session, c, sid, updated);

            return updated;
        } else {
            return current;
        }
    } else {
        return expires;
    }
}

qint64 SessionPrivate::getStoredSessionExpires(Session *session, Context *c, const QString &sessionid)
{
    const QVariant expires = session->d_ptr->store->getSessionData(c, sessionid, QStringLiteral("expires"), 0);
    return expires.toLongLong();
}

QVariant SessionPrivate::initializeSessionData(Session *session, Context *c)
{
    QVariantHash ret;
    const qint64 now = QDateTime::currentMSecsSinceEpoch() / 1000;
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
    const QVariant expires = c->stash(SESSION_EXPIRES);
    if (!expires.isNull()) {
        const QString sid = Session::id(c);
        if (!sid.isEmpty()) {
            if (Q_UNLIKELY(!m_instance)) {
                qCCritical(C_SESSION) << "Session plugin not registered";
                return;
            }

            const qint64 current  = getStoredSessionExpires(m_instance, c, sid);
            const qint64 extended = qint64(Session::expires(c));
            if (extended > current) {
                m_instance->d_ptr->store->storeSessionData(c, sid, QStringLiteral("expires"), extended);
            }
        }
    }
}

QVariant SessionPrivate::loadSessionExpires(Session *session, Context *c, const QString &sessionId)
{
    QVariant ret;
    if (c->stash(SESSION_TRIED_LOADING_EXPIRES).toBool()) {
        ret = c->stash(SESSION_EXPIRES);
        return ret;
    }
    c->setStash(SESSION_TRIED_LOADING_EXPIRES, true);

    if (!sessionId.isEmpty()) {
        const qint64 expires = getStoredSessionExpires(session, c, sessionId);

        if (expires >= QDateTime::currentMSecsSinceEpoch() / 1000) {
            c->setStash(SESSION_EXPIRES, expires);
            ret = expires;
        } else {
            deleteSession(session, c, QStringLiteral("session expired"));
            ret = 0;
        }
    }
    return ret;
}

qint64 SessionPrivate::initialSessionExpires(Session *session, Context *c)
{
    Q_UNUSED(c)
    const qint64 expires = qint64(session->d_ptr->sessionExpires);
    return QDateTime::currentMSecsSinceEpoch() / 1000 + expires;
}

qint64 SessionPrivate::calculateInitialSessionExpires(Session *session, Context *c, const QString &sessionId)
{
    const qint64 stored  = getStoredSessionExpires(session, c, sessionId);
    const qint64 initial = initialSessionExpires(session, c);
    return qMax(initial, stored);
}

qint64 SessionPrivate::resetSessionExpires(Session *session, Context *c, const QString &sessionId)
{
    const qint64 exp = calculateInitialSessionExpires(session, c, sessionId);

    c->setStash(SESSION_EXPIRES, exp);

    // since we're setting _session_expires directly, make loadSessionExpires
    // actually use that value.
    c->setStash(SESSION_TRIED_LOADING_EXPIRES, true);
    c->setStash(SESSION_EXTENDED_EXPIRES, exp);

    return exp;
}

void SessionPrivate::updateSessionCookie(Context *c, const QNetworkCookie &updated)
{
    c->response()->setCookie(updated);
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 1, 0))
void SessionPrivate::updateSessionCuteCookie(Context *c, const Cookie &updated)
{
    c->response()->setCuteCookie(updated);
}
#endif

QNetworkCookie SessionPrivate::makeSessionCookie(Session *session, Context *c, const QString &sid, const QDateTime &expires)
{
    Q_UNUSED(c)
    QNetworkCookie cookie(session->d_ptr->sessionName.toLatin1(), sid.toLatin1());
    cookie.setPath(QStringLiteral("/"));
    cookie.setExpirationDate(expires);
    cookie.setHttpOnly(session->d_ptr->cookieHttpOnly);
    cookie.setSecure(session->d_ptr->cookieSecure);
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    cookie.setSameSitePolicy(session->d_ptr->cookieSameSite);
#endif

    return cookie;
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 1, 0))
Cookie SessionPrivate::makeSessionCuteCookie(Session *session, Context *c, const QString &sid, const QDateTime &expires)
{
    Q_UNUSED(c)
    Cookie cookie(session->d_ptr->sessionName.toLatin1(), sid.toLatin1());
    cookie.setPath(QStringLiteral("/"));
    cookie.setExpirationDate(expires);
    cookie.setHttpOnly(session->d_ptr->cookieHttpOnly);
    cookie.setSecure(session->d_ptr->cookieSecure);
    cookie.setSameSitePolicy(session->d_ptr->cookieSameSite);

    return cookie;
}
#endif

void SessionPrivate::extendSessionId(Session *session, Context *c, const QString &sid, qint64 expires)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    updateSessionCookie(c, makeSessionCookie(session, c, sid, QDateTime::fromMSecsSinceEpoch(expires * 1000)));
#else
    updateSessionCuteCookie(c, makeSessionCuteCookie(session, c, sid, QDateTime::fromMSecsSinceEpoch(expires * 1000)));
#endif
}

void SessionPrivate::setSessionId(Session *session, Context *c, const QString &sid)
{
#if (QT_VERSION >= QT_VERSION_CHECK(6, 1, 0))
    updateSessionCookie(c, makeSessionCookie(session, c, sid, QDateTime::fromMSecsSinceEpoch(initialSessionExpires(session, c) * 1000)));
#else
    updateSessionCuteCookie(c, makeSessionCuteCookie(session, c, sid, QDateTime::fromMSecsSinceEpoch(initialSessionExpires(session, c) * 1000)));
#endif
}

SessionStore::SessionStore(QObject *parent)
    : QObject(parent)
{
}

#include "moc_session.cpp"
