/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef C_UTILS_LANGSELECT_H
#define C_UTILS_LANGSELECT_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>

#include <QLocale>
#include <QVector>

namespace Cutelyst {

/**
 * @defgroup plugins-utils-langselect LangSelect
 * @brief Provides a plugin to select locale based on different input parameters.
 *
 * The LangSelect plugin can set the locale (Context::setLocale()) based on different definable input parameters
 * like cookie or session values, URL query parameters and parts of the path or the domain. It will check if the
 * language requested by the user agent is @link LangSelect::setSupportedLocales() supported@endlink by the application.
 * If the language is not supported, it will use a @link LangSelect::setFallbackLocale() fallback@endlink language.
 * As another fallback it will try to get the locale from the @a Accept-Language header.
 *
 * <h3>Logging</h3>
 * Information is logged to the @c cutelyst.plugin.langselect logging category.
 *
 * <h3>Building and using</h3>
 * The plugin is linked to @c Cutelyst::Core, @c Cutelyst::Session and the QtNetwork module. To use it in your application,
 * link your application to @c Cutelyst::Utils::LangSelect. See LangSelect to learn how to use it.
 */

class Context;
class LangSelectPrivate;

/**
 * @ingroup plugins-utils-langselect
 * @class LangSelect langselect.h <Cutelyst/Plugins/Utils/LangSelect>
 * @brief Language selection plugin
 *
 * The %LangSelect plugin can be used to automatically detect and set a user's language by querying various
 * sources like session keys, cookies, path and subdomain components or the @a Accept-Language header sent by
 * the user agent (like the web browser). It will compare the detected locale against a list of locales supported
 * by the application to choose the most appropriate locale fitting the user's preferences.
 *
 * Unless the plugin has been constructed with the manual mode constructor, it will be connected to the
 * Application::beforePrepareAction() signal to set the locale. If auto detection is disabled, you can manually set the
 * locale by calling LangSelect::fromCookie(), LangSelect::fromDomain(), LangSelect::fromPath(), LangSelect::fromSession(),
 * LangSelect::fromUrlQuery() or LangSelect::fromSubDomain() at appropriate places.
 *
 * @b Note that you must register plugins like StaticSimple before the %LangSelect plugin, especially if you want to store
 * the selected locale in the domain or the path.
 *
 * On a multilingual site you will mostly have some kind of selector that allows users to choose the display language.
 * Especially on publicly available content you might want to put the locale information into the domain or URL path to
 * optimize your content for search engines.
 *
 * The plugin will also set two values to the stash that will contain the BCP47 name of the selected locale and the text
 * direction. You can set the stash keys used for this information with setLanguageCodeStashKey() and setLanguageDirStashKey().
 *
 * <h3 id="setting-supported-locales">Setting supported locales</h3>
 * One of the main purposes of this plugin is not only to select a locale, but also to select a locale that is supported
 * by your application. Therefore the plugin provides different methods to set the list of supported locales. If you already
 * use Application::loadTranslationsFromDir() or Application::loadTranslationsFromDirs() to load the translation files for
 * your application, you can simply use the returned list of that methods and give them to setSupportedLocales(). If you
 * use a different way of loading translations, have a look at the other functions to set the supported locales:
 * addSupportedLocale(), setSupportedLocales(), setLocalesFromDir() and setLocalesFromDirs().
 *
 * <h3 id="modes-of-operation">Modes of operation</h3>
 * The plugin can either work automatically or manually. The auto detection mode hooks into the
 * Application::beforePrepareAction() signal to set the locale. The mode of operation is defined when constructing
 * and registering the plugin. If auto detection is disabled, you can use one of the static functions that get and set
 * the locale. Note that you still have to set the list of supported locales and might want to set some defaults for
 * the sources like the session key, etc.
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     // register plugins like StaticSimple before the LangSelect
 *     // plugin if you use the auto detection mode
 *     auto stat = new StaticSimple(this);
 *
 *     // initializing the plugin in autodetection mode
 *     // and using the session as source and storage
 *     auto lsp = new LangSelect(this, LangSelect::Session);
 *     lsp->setSessionKey(QStringLiteral("lang"));
 * }
 * @endcode
 *
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     // initializing the plugin in manual mode
 *     // and setting a default value for the cookie name
 *     auto lsp = new LangSelect(this);
 *     lsp->setCookieName(QStringLiteral("lang"));
 * }
 * @endcode
 *
 * <h3 id="sources">Sources</h3>
 * The %LangSelect plugin supports different sources to get locale information from. Some sources only set the detected
 * locale internally, other sources that rely on the request URI will perform a redirect to set the detected source. Common
 * to all sources is, that they will fall back to the @a Accept-Language header and the locale set by setFallbackLocale()
 * if no supported locale can be detected in the source. You can omit the @a Accept-Language header and use the fallback
 * language directly by setting setDetectFromHeader() to @c false. If you set the source to LangSelect::AcceptHeader, the
 * locale will always be extracted from the @a Accept-Langauge header filed from the request and will never be stored.
 *
 * For the following examples we will assume that your application supports English, German and Portuguese and English
 * is the fallback locale.
 *
 * <h4 id="source-url-query">URL query</h4>
 * If you use LangSelect::URLQuery to register the plugin in auto mode or if you use LangSelect::fromUrlQuery() to
 * get and set the locale from the url query manually at appropriate places in you application, the plugin will try
 * to detect the locale from the query key specified for the plugin.
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     // registering the plugin in auto mode using
 *     // url query as source and setting "lang" as
 *     // query key
 *     auto lsp = new LangSelect(this, LangSelect::URLQuery);
 *     lsp->setQueryKey(QStringLiteral("lang"));
 *     lsp->setSupportedLocales({
 *                                  QLocale(QLocale::Portuguese),
 *                                  QLocale(QLocale::German),
 *                                  QLocale(QLocale::English)
 *                             });
 *     lsp->setFallbackLocale(QLocale(QLocale::English));
 * }
 * @endcode
 * If a user now requests a resource by the URL <code>https://www.example.com/my/path?lang=pt</code> the plugin will
 * automatically select Portuguese as locale and will set it to Context::setLocale(). If the use would call the URL
 * <code>https://www.example.com/my/path?lang=ru</code> the plugin would redirect him to a locale matching the Accept-Language
 * header of the browser. If that would contain some form of German or Portuguese, the redirection would be performed to
 * the language that would have the highest priority in the header. If the header would not contain a supported language,
 * the user would be redirected to <code>https://www.example.com/my/path?lang=en</code>.
 *
 * <h4 id="source-cookie">Cookie</h4>
 * If you use LangSelect::Cookie to register the plugin in auto mode or if you use LangSelect::fromCookie() to get and
 * set the locale from a cookie manually at appropriate places in your application, the plugin will try to detect the locale
 * from the cookie specified for the plugin.
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     // registering the plugin in manual mode and
 *     // setting "lang" as default cookie name
 *     auto lsp = new LangSelect(this);
 *     lsp->setCookieName(QStringLiteral("lang"));
 *     lsp->setSupportedLocales({
 *                                  QLocale(QLocale::Portuguese),
 *                                  QLocale(QLocale::German),
 *                                  QLocale(QLocale::English)
 *                             });
 *     lsp->setFallbackLocale(QLocale(QLocale::English));
 * }
 *
 * // now for example in the Auto method of the Root controller
 * // we will manually get and set the locale from the cookie
 * void Root::Auto(Contect *c)
 * {
 *     LangSelect::fromCookie(c);
 * }
 * @endcode
 * If a user now requests something on our site for the first time, there will be no cookie containing the language -
 * neither set by auto detection from header nor by some input field. So the plugin will try to detect the language
 * from the Accept-Language header and will store it to the cookie named "lang". On the next request it will not
 * have to examine the header again but can take the locale from the cookie. As the session id is also stored as a
 * cookie this approach is similar to the session approach but does not need a store for the session on the server side.
 * If you use sessions anyway, you should store the locale in the session.
 *
 * <h4 id="source-session">%Session</h4>
 * If you use LangSelect::Session to register the plugin in auto mode or if you use LangSelect::fromSession() to get and
 * set the locale from the session manually at appropriate places in your application, the plugin will try to detect the
 * locale from the session key specified for the plugin. This approach is great if you use sessions anyway because the complete
 * QLocale object can be stored in the session, making it quite fast to load compared with the other methods that have to
 * construct the QLocale again from a string on every request.
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     // registering the plugin in auto mode using
 *     // the session to store and retrieve the locale
 *     auto lsp = new LangSelect(this, LangSelect::Session);
 *     lsp->setSessionKey(QStringLiteral("lang"));
 *     lsp->setSupportedLocales({
 *                                  QLocale(QLocale::Portuguese),
 *                                  QLocale(QLocale::German),
 *                                  QLocale(QLocale::English)
 *                             });
 *     lsp->setFallbackLocale(QLocale(QLocale::English));
 *     // in this example we will disable the detection from
 *     // the Accept-Language header and will always use the
 *     // fallback language if the locale could not be loaded
 *     // from the session
 *     lsp->setDetectFromHeader(false);
 * }
 * @endcode
 * If a user now requests a resource in our application without having the locale stored in the session value identified
 * by the "lang" key, the fallback locale English will be selected and will be stored to the session. For sure you would
 * then need some selection interface that would make it possible for the user to change the display language. If you only
 * would use the session to store the locale, it might be easier to store the locale in a cookie as that would need no
 * session store.
 *
 * <h4 id="source-path">Path</h4>
 * If you use a chained dispatcher to detect the locale, you can use this plugin in manual mode and use LangSelect::fromPath()
 * at the chained action that takes the locale as path argument. This will then set the locale if it is supported or will
 * redirect to a path containing a supported locale.
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     // register the plugin in manual mode and set
 *     // the list of supported locales
 *     auto lsp = new LangSelect(this);
 *     lsp->setSupportedLocales({
 *                                  QLocale(QLocale::Portuguese),
 *                                  QLocale(QLocale::German),
 *                                  QLocale(QLocale::English)
 *                             });
 *     lsp->setFallbackLocale(QLocale(QLocale::English));
 * }
 *
 * void MyRoot::base(Context *c, const QString &locale)
 * {
 *     if (LangSelect::fromPath(c, locale) {
 *          // if the locale could be found in the
 *          // list of supported locales, it will be set,
 *          // otherwise the function will create a 307 redirect
 *          // to the same path but with a valid locale and
 *          // will call Context::detach()
 *     }
 * }
 * @endcode
 * If the user would now call the URL <code>http://www.example.com/pt/my/resource</code> the locale would be set to Portuguese
 * and the normal flow of the application would continue. If the user would call the URL <code>http://www.example.com/ru/my/resource</code>
 * and has no supported locale in the @a Accept-Language header, the function would create a redirect to
 * <code>http://www.example.com/en/my/resource</code> and would detach from the normal execution flow.
 *
 * <h4 id="source-subdomain">Subdomain</h4>
 * If you use LangSelect::SubDomain to register the plugin in auto mode or if you use LangSelect::fromSubDomain() to get the locale
 * from the subdomain manually at appropriate places in your application, the plugin will try to detect the locale from the subdomain
 * part specified for the plugin. This approach needs for sure DNS entries for every supported locale. Let us assume we have the
 * following registered subdomains: www.example.com, de.example.com, en.example.com and pt.example.com.
 * The subdomain will simply be determined by checking if the domain starts with one of the entries from the map set by
 * setSubDomainMap(). If the domain does not start with any of the map keys, the plugin will use the first domain part by splitting
 * the domain name at the dots and will try to construct a valid QLocale object from it. If there can be no supported locale found,
 * the plugin will use a locale from the @a Accept-Language header or the fallback locale. If for the example you want to use the fallback
 * language English for the www subdomain, simply set setDetectFromHeader() to @c false.
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     auto lsp = new LangSelect(this, LangSelect::SubDomain);
 *     // this will also set the list of supported locales
 *     lsp->setSubDomainMap({
 *                             {QStringLiteral("de"), QLocale(QLocale::German)},
 *                             {QStringLiteral("en"), QLocale(QLocale::English)},
 *                             {QStringLiteral("pt"), QLocale(QLocale::Portuguese)}
 *                         ]);
 *     lsp->setFallbackLocale(QLocale(QLocale::English));
 * }
 * @endcode
 * If a user now calls the URL <code>http://www.exmaple.com</code> the plugin will choose a locale from the @a Accept-Language
 * header or the fallback locale. If the user would call <code>http://pt.example.com</code> Portuguese would be set as locale.
 *
 * <h4 id="source-domain">Domain</h4>
 * If you use LangSelect::Domain to register the plugin in auto mode or if you use LangSelect::fromDomain() to get
 * the locale from the domain manually at appropriate places in your application, the plugin will try to detect the locale
 * from the domain part specified for the plugin. This approach needs for sure DNS entries for every supported locale. Let
 * us assume we have the following registered domains: exmaple.br, example.com, example.co.uk and example.de.
 * The domain will simply be determined by checking if the domain ends with one of the keys from the map set by setDomainMap().
 * If the domain does not end with any of the map keys, the plugin will try to create a valid QLocale object from the TLD of the
 * request URI. If that is not valid or not part of the supported locales, the plugin will try to detect the locale from the
 * @a Accept-Language header (if setDetectFromHeader() has not been set to @c false) or will use the fallback language.
 * @code{.cpp}
 * bool MyApp::init()
 * {
 *     auto lsp = new LangSelect(this, LangSelect::Domain);
 *     // this will also set the list of supported locales
 *     lsp->setDomainMap({
 *                         {QStringLiteral("br"), QLocale(QLocale::Portuguese)},
 *                         {QStringLiteral("uk"), QLocale(QLocale::English, QLocale::UnitedKingdom)},
 *                         {QStringLiteral("de"), QLocale(QLocale::German)}
 *                      });
 *     lsp->setFallbackLocale(QLocale(QLocale::English));
 * }
 * @endcode
 * If a user now calls the URL <code>http://www.examble.br</code> the locale will be set to Portuguese and the normal operation
 * flow will continue. If there are also domains pointing to your application that are not part of the domain map
 *
 * <h3 id="links-and-locales">Links and locales</h3>
 * If you detect and set locales based on the session or a cookie you do not need to change anything on your links in your application.
 * If you use the path or query to detect the locale, you can use Context::uriFor() or Context::uriForAction() to set the locale on
 * internal URIs. For Grantlee themes there is also the %Cutelyst specific tag @c c_uri_for that can be used as
 * <code>{% c_uri_for "/path" "arg1" "arg2" QUERY c.request.queryParams "foo=bar" %}</code>. Taking the defaul name of the stash key
 * with the BCP47 name of the selected locale and using the path approach to set the locale, you could use the tag as follows:
 * <code>{% c_uri_for "/path" c_langselect_lang "otherArg" QUERY c.request.queryParams "foo=bar" %}</code>. If you use the domain
 * or subdomain to set the locale, simply use relative paths in your internal links.
 *
 * @since %Cutelyst 2.1.0
 */
class CUTELYST_PLUGIN_UTILS_LANGSELECT_EXPORT LangSelect : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LangSelect)
public:
    /**
     * Sources that can be used for automatically detecting the locale.
     */
    enum Source : quint8 {
        URLQuery     = 0,   /**< Tries to get and store the locale from an URL query parameter. Requires setQueryKey(). */
        Session      = 1,   /**< Tries to get and store the locale from a session key. Requires setSessionKey(). */
        Cookie       = 2,   /**< Tries to get and store the locale from a cookie. Requires setCookieName(). */
        SubDomain    = 3,   /**< Tries to get and store the locale from a subdomain part. Requires setSubDomainMap(). */
        Domain       = 4,   /**< Tries to get and store the locale from the domain. Requires setDomainMap(). */
        AcceptHeader = 254, /**< Will the locale only detect from the Accept-Header and will not store it. */
        Fallback     = 255  /**< Only used internal. */
    };
    Q_ENUM(Source)

    /**
     * Constructs a new %LangSelect object with the given @a parent and @a source in
     * <b>auto detection mode</b>.
     *
     * The plugin will be connected to the Application::beforePrepareAction signal
     * and will automatically set the appropriate locale extracted from @a source.
     *
     * This mode is good is good if you use the same approach to detect and set the locale
     * in your complete application.
     */
    LangSelect(Application *parent, Source source);

    /**
     * Constructs a new %LangSelect object with the given @a parent in <b>manual mode</b>.
     *
     * The plugin will @b not be connected to the Application::beforePrepareAction signal
     * so you have to use one of the static functions to set and store the locale.
     */
    LangSelect(Application *parent);

    /**
     * Deconstructs the %LangSelect object.
     */
    virtual ~LangSelect() override;

    /**
     * Sets the list of supported @a locales.
     * Use this function if you exactly now which locales your application supports. It also fits
     * perfectly to the list of loaded locales that is returned by Application::loadTranslations()
     * or Application::loadTranslationsFromDirs().
     * @sa addSupportedLocale(), setLocalesFromDir(), setLocalesFromDirs()
     */
    void setSupportedLocales(const QVector<QLocale> &locales);

    /**
     * Sets the list of supported @a locales.
     * Use this function if you exactly now which locales your application supports.
     * Only valid locale strings are added to the list of supported locales.
     * @sa addSupportedLocale(), setLocalesFromDir(), setLocalesFromDirs()
     */
    void setSupportedLocales(const QStringList &locales);

    /**
     * Adds a single @c locale to the list of supported locales.
     * @sa setSupportedLocales(), setLocalesFromDir(), setLocalesFromDirs()
     */
    void addSupportedLocale(const QLocale &locale);

    /**
     * Adds a single @c locale to the list of supported locales.
     * The locale string will only be added if it is valid.
     * @sa setSupportedLocales(), setLocalesFromDir(), setLocalesFromDirs()
     */
    void addSupportedLocale(const QString &locale);

    /**
     * Sets the list of supported locales by reading translation files from @a path. @a name specifies
     * the base file name while @a prefix is a delimeter used to delimit the base name from the locale identification
     * part. @a suffix is the file suffix, for Qt translations it will be mostly @a ".qm".
     *
     * You can use this function if your translations are organized in a single directory like
     * @li @c /usr/share/myapp/translations/myapp_de.qm
     * @li @c /usr/share/myapp/translations/myapp_pt_BR.qm
     * @li @c ...
     *
     * For the above organization of translation files, the call to this function would be as follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *     auto lsp = new LangSelect(this);
     *     lsp->setLocalesFromDir(QStringLiteral("/usr/share/myapp/translations"), QStringLiteral("myapp"), QStringLiteral("_"));
     * }
     * @endcode
     *
     * If you already use Application::loadTranslations() to load your app translations you don't need to use this function.
     * Simply take the list returned by Application::loadTranslations() and give it to setSupportedLocales().
     */
    void setLocalesFromDir(const QString &path, const QString &name, const QString &prefix = QStringLiteral("."), const QString &suffix = QStringLiteral(".qm"));

    /**
     * Sets the list of supported locales by reading translation files with a specific @a name
     * from a directory structure under @a path.
     *
     * You can use this function if your translations are organized in multiple directories like
     * @li @c /usr/share/locale/de/LC_MESSAGES/myapp.qm
     * @li @c /usr/share/locale/pt_BR/LC_MESSAGES/myapp.qm
     * @li @c ...
     *
     * For the above organization of translation files, the call to this function would be as follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *     auto lsp = new LangSelect(this);
     *     lsp->setLocalesFromDirs(QStringLiteral("/usr/share/locale"), QStringLiteral("LC_MESSAGES/myapp.qm"));
     * }
     * @endcode
     *
     * If you already use Application::loadTranslationFromDirs() to load your app translations you don't need to use this function.
     * Simply take the list returned by Application::loadTranslationsFromDirs() and give it to setSupportedLocales().
     */
    void setLocalesFromDirs(const QString &path, const QString &name);

    /**
     * Returns the list of supported locales. There is also the static method LangSelect::getSupportedLocales().
     * @sa setSupportedLocales(), addSupportedLocale(), setLocalesFromDir(), setLocalesFromDirs()
     */
    QVector<QLocale> supportedLocales() const;

    /**
     * Sets a @a key used in the URL query to store and retrieve the locale.
     */
    void setQueryKey(const QString &key);

    /**
     * Set a session @a key used to store and retrieve the locale.
     */
    void setSessionKey(const QString &key);

    /**
     * Sets the @a name of a cookie to store and retrieve the locale.
     */
    void setCookieName(const QString &name);

    /**
     * Sets the @a map for subdomains as source for locale selection. The @a map should contain
     * the locale subdomain part as key and the associated locale as value. See the <a href="#source-subdomain">
     * subdomain example</a> to learn more about this approach.
     */
    void setSubDomainMap(const QMap<QString, QLocale> &map);

    /**
     * Sets the @a map for full domains as source for locale selection. The @a map should contain
     * the locale domain part as key and the associated locale as value. See the <a href="#source-domain">
     * domain example</a> to learn more about this approach.
     */
    void setDomainMap(const QMap<QString, QLocale> &map);

    /**
     * Sets the @a fallback locale to be used if no appropriate locale can be found.
     */
    void setFallbackLocale(const QLocale &fallback);

    /**
     * Set to @c false if locale detection by @a Accept-Language header should be disabled.
     * By default, the detection by the header is enabled if the other sources can not extract
     * a valid supported locale.
     */
    void setDetectFromHeader(bool enabled);

    /**
     * Sets the name of the stash @a key that will contain the BCP47 name of the selected locale.
     */
    void setLanguageCodeStashKey(const QString &key = QStringLiteral("c_langselect_lang"));

    /**
     * Sets the name of the stash @a key that will contain the text direction of the selected locale,
     * either "ltr" or "rtl".
     */
    void setLanguageDirStashKey(const QString &key = QStringLiteral("c_langselect_dir"));

    /**
     * Returns the list of supported locales.
     * @sa setSupportedLocales(), addSupportedLocale(), setLocalesFromDir(), setLocalesFromDirs()
     */
    static QVector<QLocale> getSupportedLocales();

    /**
     * Tries to get the locale from an URL query parameter specified by @a key. If that key is not present
     * or does not contain a <a href="#setting-supported-locales">supported locale</a>, the plugin will either
     * try to get a supported locale from the @a Accept-Language header (if setDetectFromHeader() is not set
     * to @c false) or it will use the locale set by setFallbackLocale().
     *
     * If the URL query did not contain a valid supported locale, the locale detected by the header or the fallback
     * locale will be set to the URL query by creating a 307 redirect on the response that contains the selected
     * locale in the query @a key. If this redirect will be performed, the function returns @c false, otherwise
     * it will return @c true.
     *
     * See the <a href="#source-url-query">URL query example</a> to learn more about this approach.
     *
     * @param c     The current Context.
     * @param key   The URL query key to check. If empty the value set by setQueryKey() will be used.
     * @return @c true if the URL query contained a supported locale, @c false if the locale will be set by a redirect.
     */
    static bool fromUrlQuery(Context *c, const QString &key = QString());

    /**
     * Tries to get the locale from a session value specified by @a key. If that key is not present or does not
     * contain a <a href="#setting-supported-locales">supported locale</a>, the plugin will either
     * try to get a supported locale from the @a Accept-Language header (if setDetectFromHeader() is not set
     * to @c false) or it will use the locale set by setFallbackLocale().
     *
     * If the session did not contain a valid supported locale, the locale detected by the header or the fallback
     * locale will be set to the session under the given @a key.
     *
     * See the <a href="#source-session">session example</a> to learn more about this appraoch.
     *
     * @param c     The current Context.
     * @param key   The session key to get and store the locale. If empty the value set by setSessionKey() will be used.
     * @return @c true if the session contained a supported locale, otherwise @c false.
     */
    static bool fromSession(Context *c, const QString &key = QString());

    /**
     * Tries to get the locale from a cookie specified by @a name. If that cookie is not present or does not contain
     * a <a href="#setting-supported-locales">supported locale</a>, the plugin will either
     * try to get a supported locale from the @a Accept-Language header (if setDetectFromHeader() is not set
     * to @c false) or it will use the locale set by setFallbackLocale().
     *
     * If the cookie did not contain a valid locale, the locale detected by the header or the fallback
     * locale will be set to the cookie with the given @a name.
     *
     * See the <a href="#source-cookie">cookie example</a> to learn more about this approach.
     *
     * @param c     The current Context.
     * @param name  The name of the cookie to get and store the locale. If empty, the value set by setCookieName() will be used.
     * @return @c true if the cookie contained a supported locale, otherwise @c false.
     */
    static bool fromCookie(Context *c, const QString &name = QString());

    /**
     * Tries to get the locale from the subdomain specified in the @a subDomainMap. The @a subDomainMap has to contain
     * mappings between subdomains and locales. See the <a href="#source-subdomain">subdomain example</a> to get an idea
     * on how this approach works. If the domain in the request URI can nat be found in the @a subDomainMap, the plugin
     * will try to create a valid QLocale object from the first subdomain part. If that is not valid it will either
     * try to get a supported locale from the @a Accept-Language header (if setDetectFromHeader() is not set to @c false)
     * or it will use the locale set by setFallbackLocale().
     *
     * @param c             The current Context.
     * @param subDomainMap  Mapping between subdomains and locales. If empty, the map set by setSubDomainMap() will be used.
     * @return @c true if the subdomain has been found in the @a subDomainMap or the first sub domain, @c false if the locale
     * will be set by a the @a Accept-Header or the fallback locale.
     */
    static bool fromSubDomain(Context *c, const QMap<QString, QLocale> &subDomainMap = QMap<QString, QLocale>());

    /**
     * Tries to get the locale from the domain specified in the @a domainMap. The @a domainMap has to contain
     * mappings between domains and locales. See the <a href="#source-domain">domain example</a> to get an idea on
     * how this approach works. If the domain in the request URI can not be found in the @a domainMap, the plugin
     * will try to create a valid QLocale object from the TLD part of the domain. If that is not valid, it will
     * either try to get a supported locale from the @a Accept-Language header (if setDetectFromHeader() is not
     * set to @c false) or it will use the locale set by setFallbackLocale().
     *
     * @param c         The current Context.
     * @param domainMap Mapping between domains and locales. If empty, the map set by setDomainMap() will be used.
     * @return @c true if the domain has been found in the @a domainMap or the TLD, otherwise @c false if the locale
     * has been set by the @a Accept-Header of the fallback locale.
     */
    static bool fromDomain(Context *c, const QMap<QString, QLocale> &domainMap = QMap<QString, QLocale>());

    /**
     * Takes the @a locale from a string extracted as argument from a chained action. See the <a href="#source-path">
     * path example</a> to get an idea on how this approach works.
     *
     * I the @a locale is not part of the supported locales, the plugin will create a 307 redirect to the same path
     * but with a valid locale either extracted from the @a Accept-Language header (if setDetectFromHeader() is not
     * set to @c false) or it will use the locale set by setFallbackLocale(). If the redirect will be performed, the
     * function will return @c false and Context::detach() will be called, otherwise it will return @c true.
     *
     * @param c         The current Context.
     * @param locale    Locale string supported by QLocale.
     * @return @c true if the locale has been found in the path, @c false if the locale will be set by a redirect to
     * a path containing a valid locale.
     */
    static bool fromPath(Context *c, const QString &locale);

protected:
    /**
     * Sets the plugin up and checks the plugin configuration. If the configuration contains errors,
     * it will return @c false, otherwise it will return @c true. If the plugin has been constructed
     * with auto detection constructor, it will connect the plugin to the Application::beforePrepareAction()
     * signal.
     */
    virtual bool setup(Application *app) override;

private:
    LangSelectPrivate *const d_ptr;
};

} // namespace Cutelyst

#endif // C_UTILS_LANGSELECT_H
