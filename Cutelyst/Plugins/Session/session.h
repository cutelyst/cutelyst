/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CSESSION_H
#define CSESSION_H

#include <Cutelyst/Plugins/Session/cutelyst_plugin_session_export.h>
#include <Cutelyst/plugin.h>

#include <QVariant>

/**
 * @ingroup plugins
 * @defgroup plugins-session Session
 * @brief Plugins and classes to store and manage user sessions.
 */

namespace Cutelyst {

class Context;
/**
 * @ingroup plugins-session
 * @headerfile "" <Cutelyst/Plugins/Session/Session>
 * @brief Abstract class to create a session store.
 *
 * Use this class to create your own session store to use with the Session plugin. Reimplement
 * the pure virtual functions getSessionData(), storeSessionData(), deleteSessionData() and
 * deleteExpiredSessions() in your derived class.
 *
 * %Cutelyst already ships with some session stores to store user sessions in the
 * @link SessionStoreFile filesystem@endlink or on @link MemcachedSessionStore memcached@endlink
 * servers.
 */
class CUTELYST_PLUGIN_SESSION_EXPORT SessionStore : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new %SessionStore object with the given @a parent.
     */
    explicit SessionStore(QObject *parent = nullptr);

    /**
     * Returns the session data for the given session id @a sid and @a key, if key does not exist
     * returns @a defaultValue.
     */
    virtual QVariant getSessionData(Context *c,
                                    const QByteArray &sid,
                                    const QString &key,
                                    const QVariant &defaultValue = QVariant()) = 0;

    /**
     * Stores the session data for the given session id @a sid and @a key to @a value.
     */
    virtual bool storeSessionData(Context *c,
                                  const QByteArray &sid,
                                  const QString &key,
                                  const QVariant &value) = 0;

    /**
     * Removes all session data for the given session id @a sid and @a key.
     */
    virtual bool deleteSessionData(Context *c, const QByteArray &sid, const QString &key) = 0;

    /**
     * Removes all expired sessions which are above @a expires.
     */
    virtual bool deleteExpiredSessions(Context *c, quint64 expires) = 0;
};

class SessionPrivate;
/**
 * @ingroup plugins-session
 * @headerfile "" <Cutelyst/Plugins/Session/Session>
 * @brief %Plugin providing methods for session management.
 *
 * The %Session plugin manages user sessions and uses a SessionStore to store the session
 * data. %Cutelyst already ships with session stores to store sessions in the
 * @link SessionStoreFile filesystem@endlink and on @link MemcachedSessionStore memcached@endlink
 * servers. You can create your own session store by creating a new subclass of SessionStore
 * and set it to this plugin using setStorage().
 *
 * By default, if no session store has been manually set, a SessionStoreFile will be used.
 *
 * <H3>Usage example</H3>
 *
 * @code{.cpp}
 * #include <Cutelyst/Plugins/Session/Session>
 * #include <Cutelyst/Plugins/Session/sessionstorefile.h>
 *
 * bool MyCutelystApp::init()
 * {
 *      // other initiaization stuff
 *      // ...
 *
 *      auto sess = new Session(this);
 *      sess->setStorage(std::make_unique<SessionStoreFile>(sess));
 *
 *      // maybe more initalization stuff
 *      // ...
 * }
 *
 * void MyController::myMethod(Context *c)
 * {
 *      Session::setValue(QStringLiteral("myKey"), QStringLiteral("myValue"));
 *
 *      // ...
 * }
 *
 * void MyOtherController::myOtherMethod(Context *c)
 * {
 *      const QString myValue = Session::value(QStringLiteral("myKey")).toString();
 *
 *      // ...
 * }
 * @endcode
 *
 * <H3 id="configfile">Configuration file options</H3>
 *
 * There are some options you can set in your \ref configuration "application configuration file"
 * in the @c Cutelyst_Session_Plugin section. You can set your own default values using the
 * @a defaultConfig parameter of the overloaded constructor.
 *
 * @configblock{expires,integer or string,2 hours}
 * The expiration duration of the session. The value will be parsed by Utils::durationFromString()
 * (since %Cutelyst 4.0.0, before that, it took simple seconds), so you can use one of the
 * supported human readable time spans.
 * @endconfigblock
 *
 * @configblock{verify_address,bool,false}
 * If @c true, the plugin will check if the IP address of the requesting user matches the
 * address stored in the session data. In case of a mismatch, the session will be deleted.
 * @endconfigblock
 *
 * @configblock{verify_user_agent,bool,false}
 * If @c true, the plugin will check if the user agent of the requesting user matches the user
 * agent stored in the session data. In case of a mismatch, the session will be deleted.
 * @endconfigblock
 *
 * @configblock{cookie_http_only,bool,true}
 * If @c true, the session cookie will have the httpOnly flag set so that the cookie is not
 * accessible to JavaScript’s Document.cookie API.
 * @endconfigblock
 *
 * @configblock{cookie_secure,bool,false}
 * If @c true, the session cookie will have the secure flag set so that the cookie is only
 * sent to the server with an encrypted request over the HTTPS protocol.
 * @endconfigblock
 *
 * @configblock{cookie_same_site,string,strict,default\,none\,lax\,strict}
 * Defines the SameSite attribute of the session cookie. See <A
 * HREF="https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Set-Cookie/SameSite">MDN</A> to
 * learn more about SameSite cookies. Also have a look at QNetworkCookie::SameSite. This
 * configuration key is available since %Cutelyst 3.8.0.
 * @endconfigblock
 *
 * @logcat{plugin.session}
 */
class CUTELYST_PLUGIN_SESSION_EXPORT Session : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Session)
public:
    /**
     * Constructs a new %Session object with the given @a parent.
     */
    Session(Application *parent);

    /**
     * Constructs a new %Session object with the given @a parent and @a defaultConfig.
     *
     * Use the @a defaultConfig to set default values for the configuration entries from
     * the <A HREF="#configfile">configuration file</A>.
     *
     * @since %Cutelyst 4.0.0
     */
    Session(Application *parent, const QVariantMap &defaultConfig);

    /**
     * Destroys the %Session object.
     */
    virtual ~Session();

    /**
     * Sets up the plugin and loads the configuration.
     */
    virtual bool setup(Application *app) final;

    /**
     * Sets the session @a store. If no @a store has been set manually,
     * a SessionStoreFile will be created.
     */
    void setStorage(std::unique_ptr<SessionStore> store);

    /**
     * Returns the session storage.
     */
    SessionStore *storage() const;

    /**
     * Returns the current session id or null if
     * there is no current session
     */
    static QByteArray id(Context *c);

    /**
     * This method returns the time when the current session will expire, or 0 if there is no
     * current session. If there is a session and it already expired, it will delete the session and
     * return 0 as well.
     */
    static quint64 expires(Context *c);

    /**
     * Change the session expiration time for this session
     *
     * Note that this only works to set the session longer than the config setting.
     */
    static void changeExpires(Context *c, quint64 expires);

    /**
     * This method is used to invalidate a session. It takes an optional @a reason parameter
     * which will be saved in deleteReason if provided.
     *
     * @note This method will also delete your flash data.
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
     * Returns the value for session @a key. If the session key doesn't exist, returns
     * @a defaultValue.
     */
    static QVariant
        value(Context *c, const QString &key, const QVariant &defaultValue = QVariant());

    /**
     * Sets the value for session @a key to @a value. If the key already exists, the previous
     * value is overwritten.
     */
    static void setValue(Context *c, const QString &key, const QVariant &value);

    /**
     * Removes the session @a key.
     */
    static void deleteValue(Context *c, const QString &key);

    /**
     * Removes all session @a keys.
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
