/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CSESSION_H
#define CSESSION_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>

#include <QVariant>

namespace Cutelyst {

class Context;
class CUTELYST_PLUGIN_SESSION_EXPORT SessionStore : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new session store object with the given parent.
     */
    explicit SessionStore(QObject *parent = nullptr);

    /**
     * Returns the session data for the given session id sid and key, if key does not exist returns defaultValue.
     */
    virtual QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue = QVariant()) = 0;

    /**
     * Stores the session data for the given session id sid and key to value.
     */
    virtual bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) = 0;

    /**
     * Removes all session data for the given session id sid and key.
     */
    virtual bool deleteSessionData(Context *c, const QString &sid, const QString &key) = 0;

    /**
     * Removes all expired sessions which are above expires.
     */
    virtual bool deleteExpiredSessions(Context *c, quint64 expires) = 0;
};

class SessionPrivate;
/**
 * Plugin providing methods for session management.
 *
 * <H3>Configuration file options</H3>
 *
 * There are some options you can set in your application configuration file in the @c Cutelyst_Session_Plugin section.
 *
 * @par expires
 * @parblock
 * Integer value, default: 7200
 *
 * Expiration duration of the session in seconds.
 * @endparblock
 *
 * @par verify_address
 * @parblock
 * Boolean value, default: false
 *
 * If enabled, the plugin will check if the IP address of the requesting user matches the
 * address stored in the session data. In case of a mismatch, the session will be deleted.
 * @endparblock
 *
 * @par verify_user_agent
 * @parblock
 * Boolean value, default: false
 *
 * If true, the plugin will check if the user agent of the requesting user matches the user
 * agent stored in the session data. In case of a mismatch, the session will be deleted.
 * @endparblock
 *
 * @par cookie_http_only
 * @parblock
 * Boolean value, default: true
 *
 * If true, the session cookie will have the httpOnly flag set so that the cookie is not
 * accessible to JavaScript's Document.cookie API.
 * @endparblock
 *
 * @par cookie_secure
 * @parblock
 * Boolean value, default: false
 *
 * If true, the session cookie will have the secure flag set so that the cookie is only
 * sent to the server with an encrypted request over the HTTPS protocol.
 * @endparblock
 *
 * @par cookie_same_site
 * @parblock
 * String value, default: strict; acceptable values: default, none, lax, strict
 *
 * Defines the SameSite attribute of the session cookie. See <A HREF="https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie/SameSite">MDN</A>
 * to learn more about SameSite cookies. This configuration key is available since
 * Cutelyst 3.8.0 and is only available if Cutelyst is compiled against Qt 6.1.0 or newer.
 * @endparblock
 */
class CUTELYST_PLUGIN_SESSION_EXPORT Session : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Session)
public:
    /**
     * Constructs a new session object with the given parent.
     */
    Session(Application *parent);
    virtual ~Session();

    /**
     * Sets up the plugin and loads the configuration.
     */
    virtual bool setup(Application *app) final;

    /**
     * Sets the session storage
     */
    void setStorage(SessionStore *store);

    /**
     * Returns the session storage
     */
    SessionStore *storage() const;

    /**
     * Returns the current session id or null if
     * there is no current session
     */
    static QString id(Context *c);

    /**
     * This method returns the time when the current session will expire, or 0 if there is no current session.
     * If there is a session and it already expired, it will delete the session and return 0 as well.
     */
    static quint64 expires(Context *c);

    /**
     * change the session expiration time for this session
     *
     * Note that this only works to set the session longer than the config setting.
     */
    static void changeExpires(Context *c, quint64 expires);

    /**
     * This method is used to invalidate a session. It takes an optional parameter
     * which will be saved in deleteReason if provided.
     *
     * NOTE: This method will also delete your flash data.
     */
    static void deleteSession(Context *c, const QString &reason = QString());

    /**
     * This method contains a string with the reason a session was deleted. Possible values include:
     * - session expired
     * - address mismatch
     * - user agent mismatch
     */
    static QString deleteReason(Context *c);

    /**
     * Returns the value for session key. If the session key doesn't exist, returns defaultValue.
     */
    static QVariant value(Context *c, const QString &key, const QVariant &defaultValue = QVariant());

    /**
     * Sets the value for session key to value. If the key already exists, the previous value is overwritten.
     */
    static void setValue(Context *c, const QString &key, const QVariant &value);

    /**
     * Removes the session key.
     */
    static void deleteValue(Context *c, const QString &key);

    /**
     * Removes all session keys.
     */
    static void deleteValues(Context *c, const QStringList &keys);

    /**
     * Returns true if the session is valid.
     */
    static bool isValid(Context *c);

protected:
    SessionPrivate *d_ptr;

private:
    Q_PRIVATE_SLOT(d_func(), void _q_saveSession(Context *))
};

} // namespace Cutelyst

#endif // CSESSION_H
