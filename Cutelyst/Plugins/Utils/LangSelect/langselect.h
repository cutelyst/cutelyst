/*
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef C_UTILS_LANGSELECT_H
#define C_UTILS_LANGSELECT_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>
#include <QVector>
#include <QLocale>

namespace Cutelyst {

/**
 * @defgroup plugins-utils-langselect LangSelect
 * @brief Provides a plugin to select locale based on different input parameters.
 *
 * The LangSelect plugin can set the locale (Context::setLocale()) based on different definable input parameters
 * like cookie or session values, URL query parameters and parts of the path or the domain. It will check if the
 * language requested by the user agent is @link LangSelect::setSupportedLocales() supported@endlink by the application.
 * If the language is not supported, it will use a @link LangSelect::setFallbackLocale() fallback@endlink language.
 * The plugin can try multiple sources to detect the locale, as last resort it will try to get the locale from the
 * @a Accept-Language header.
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
 * Unless the plugin has been constructed with @a autoDetect set to @c false, it will be connected to the
 * Application::beforePrepareAction signal to set the locale. If auto detection is disabled, you can manually set the
 * locale by calling LangSelect::select() on appropriate places.
 *
 * @b Note that you must register plugins like StaticSimple before the %LangSelect plugin, especially if you want to store
 * the selected locale in the domain or the path.
 *
 * On a multilingual site you will mostly have some kind of selector that allows users to choose the display language.
 * Especially on publicly available content you will put the locale information into the domain or URL path to optimize
 * your content for search engines.
 *
 * <h3>Examples</h3>
 * <h4>Read locale from URL query</h4>
 * This is a rather classic approach of setting the locale
 *
 * @since Cutelyst 2.0.0
 */
class CUTELYST_PLUGIN_UTILS_LANGSELECT_EXPORT LangSelect : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(LangSelect)
public:
    /**
     * Sources for the locale detection.
     */
    enum Source : quint8 {
        URLQuery        = 0,    /**< Tries to get the locale from an URL query parameter. */
        Session         = 1,    /**< Tries to get the locale from a session key. */
        Cookie          = 2,    /**< Tries to get the locale from a cookie. */
        SubDomain       = 3,    /**< Tries to get the locale from a subdomain part. */
        Domain          = 4,    /**< Tries to get the locale from the domain. */
        Path            = 5,    /**< Tries to get the locale from an URL path part. */
        AcceptHeader    = 254,  /**< Only used internal. */
        Fallback        = 255   /**< Only used internal. */
    };

    /**
     * Constructs a new %LangSelect object with the given @a parent.
     *
     * If @a autoDetect is set to @c true, the plugin will be connected to the
     * Application::beforePrepareAction signal and will automatically set
     * the appropriate locale. This mode is good if you use the same approach to
     * detect and set the locale in your complete application. If you have different parts
     * with different approaches, it might be better to disable @a autoDetect and use
     * LangSelect::select() at appropriate places.
     */
    LangSelect(Application *parent, bool autoDetect = true);

    /**
     * Deconstructs the %LangSelect object.
     */
    virtual ~LangSelect();

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
    void setLocalesFromDir(const QString &path, const QString &name, const QString prefix = QLatin1String("."), const QString &suffix = QLatin1String(".qm"));

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
     * Sets the @a key used in the URL query to specify the locale.
     */
    void setQueryKey(const QString &key);

    /**
     * Set the session @a key used to store and retrieve the locale.
     */
    void setSessionKey(const QString &key);

    /**
     * Sets the @a name of the cookie to store and retrieve the locale.
     */
    void setCookieName(const QString &name);

    /**
     * Sets the zero-based @a index of the sub domain part used to store and retrieve the locale.
     * Values below @c 0 will disable this source.
     */
    void setSubDomainIndex(qint8 index);

    /**
     * Sets the zero-based @a index of the URL path part used to store and retrieve the locale.
     * Values below @c 0 will disable this source.
     */
    void setPathIndex(qint8 index);

    /**
     * Sets the @a map for full domain as source for locale selection. The @a map should contain
     * the common domain part as key and the associated locale as value.
     *
     * <h3>Example</h3>
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *     auto lsp = new LangSelect(this);
     *     lsp->setDomainMap({
     *                          {QStringLiteral("example.br"), QLocale(QLocale::Portuguese, QLocale::Brazil)},
     *                          {QStringLiteral("example.de"), QLocale(QLocale::German, QLocale::Germany)}
     *                      });
     * }
     * @endcode
     */
    void setDomainMap(const QMap<QString,QLocale> &map);

    /**
     * Sets the order of the sources.
     */
    void setSourceOrder(const QVector<Source> &order);

    /**
     * Sets the fallback language to be used if no appropriate locale can be found.
     */
    void setFallbackLocale(const QLocale &fallback);

    /**
     * Returns the list of supported locales.
     * @sa setSupportedLocales(), addSupportedLocale(), setLocalesFromDir(), setLocalesFromDirs()
     */
    static QVector<QLocale> getSupportedLocales();

    /**
     * Tries to detect and set the locale based on the @a sourceOrder. Use this function if you
     * added the plugin with @a autoDetect set to @c false in the constructor. It uses the same
     * methods to detect and set the locale as the automatic way but can be used more flexible
     * if you use different approaches for gettings and setting the locale in your application.
     * This will return @c true if the locale will be set by redirecting to a different path or
     * subdomain.
     */
    static bool select(Context *c, const QVector<Source> &sourceOrder);

protected:
    /**
     * Sets the plugin up.
     */
    virtual bool setup(Application *app) override;

private:
    LangSelectPrivate *const d_ptr;
};

}

#endif // C_UTILS_LANGSELECT_H
