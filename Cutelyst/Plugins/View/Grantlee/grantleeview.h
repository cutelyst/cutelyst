/*
 * Copyright (C) 2013-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef GRANTLEE_VIEW_H
#define GRANTLEE_VIEW_H

#include <QObject>
#include <QStringList>
#include <QLocale>
#include <QVector>

#include <Cutelyst/View>

class QTranslator;

namespace Grantlee {
class Engine;
}

namespace Cutelyst {

class GrantleeViewPrivate;
/**
 * GrantleeView is a Cutelyst::View handler that renders templates
 * using Grantlee engine.
 *
 * This View also exports a Grantlee tag for dealing with
 * Cutelyst::Context::uriFor():
 *
 * {% c_uri_for "/path" "arg1" "arg2" QUERY "foo=bar" c.req.queryParams %}
 *
 * Where only the path is required, and QUERY keyword must preceed query parameters
 */
class CUTELYST_VIEW_GRANTLEE_EXPORT GrantleeView : public View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GrantleeView)
public:
    /*!
     * Constructs a GrantleeView object with the given parent and name.
     */
    explicit GrantleeView(QObject *parent = nullptr, const QString &name = QString());

    Q_PROPERTY(QStringList includePaths READ includePaths WRITE setIncludePaths NOTIFY changed)
    /*!
     * Returns the list of include paths
     */
    QStringList includePaths() const;

    /*!
     * Sets the list of include paths which will be looked for when resolving templates files
     */
    void setIncludePaths(const QStringList &paths);

    Q_PROPERTY(QString templateExtension READ templateExtension WRITE setTemplateExtension NOTIFY changed)
    /*!
     * Returns the template extension
     */
    QString templateExtension() const;

    /*!
     * Sets the template extension, defaults to ".html"
     */
    void setTemplateExtension(const QString &extension);

    Q_PROPERTY(QString wrapper READ wrapper WRITE setWrapper NOTIFY changed)

    /*!
     * Returns the template wrapper.
     */
    QString wrapper() const;

    /*!
     * Sets the template wrapper name, the template will be rendered into
     * content variable in which the wrapper template should render.
     */
    void setWrapper(const QString &name);

    Q_PROPERTY(bool cache READ isCaching WRITE setCache NOTIFY changed)
    /*!
     * Returns true if caching is enabled
     */
    bool isCaching() const;

    /*!
     * Sets if template caching should be done, this increases
     * performance at the cost of higher memory usage.
     */
    void setCache(bool enable);

    /**
     * Returns the Grantlee::Engine pointer that is used by this engine.
     */
    Grantlee::Engine *engine() const;

    /**
     * When called cache is set to true and templates are loaded.
     */
    void preloadTemplates();

    QByteArray render(Context *c) const final;

    /**
     * Adds a \a translator for the specified \a locale to the list of translators.
     *
     * \par Example usage
     * \code{.cpp}
     * bool MyCutelystApp::init()
     * {
     *      // ...
     *
     *      auto view = new GrantleeView(this);
     *
     *      auto deDeTrans = new QTranslator(this);
     *      if (deDeTrans->load(QStringLiteral("de_DE"), QStringLiteral("/path/to/my/translations")) {
     *          view->addTranslator(QLocale("de_DE"), deDeTrans);
     *      }
     *
     *      auto ptBrTrans = new QTranslator(this);
     *      if (ptBrTrans->load(QStringLiteral("pt_BR"), QStringLiteral("/path/to/my/translations")) {
     *          view->addTranslator(QLocale("pt_BR"), ptBrTrans);
     *      }
     *
     *      // ...
     * }
     * \endcode
     *
     * \sa loadTranslationsFromDir()
     *
     * \since Cutelyst 1.5.0
     */
    void addTranslator(const QLocale &locale, QTranslator *translator);

    /**
     * Adds a \a translator for the specified \a locale to the list of translators.
     *
     * The \a locale string should be parseable by QLocale.
     *
     * \overload
     *
     * \sa loadTranslationsFromDir()
     *
     * \since Cutelyst 1.4.0
     */
    void addTranslator(const QString &locale, QTranslator *translator);

    /**
     * Dynamically adds translation \a catalog at \a path to the translator.
     *
     * Translation catalogs can be used to dynamically integrate translations into the
     * GrantleeView, for example for plugins and themes. The \a catalog could be the name
     * of an extension for example that is loaded from a locale specifc directory under \a path.
     *
     * The catalog will be loaded in the following way: /path/locale/catalog, for example
     * \c /usr/share/mycutelystapp/l10n/de_DE/fancytheme.qm. The current locale is defined by
     * Context::locale() when rendering the theme. The \a path \c /usr/share/myapp/l10n would
     * then contain locale specific subdirectories like de_DE, pt_BR, etc. that contain the
     * translation files named by \a catalog.
     *
     * \par Usage example:
     * \code{.cpp}
     * bool MyCutelystApp::init()
     * {
     *      // ...
     *
     *      auto view = new GrantleeView(this);
     *      view->addTranslationCatalog(QStringLiteral("/usr/share/mycutelystapp/l10n"), QStringLiteral("fancytheme"));
     *
     *      // ...
     * }
     * \endcode
     *
     * \since Cutelyst 1.5.0
     */
    void addTranslationCatalog(const QString &path, const QString &catalog);

    /**
     * Adds a dictionary of translation catalogs and paths to the translator.
     *
     * The \a key of the QHash is the name of the catalog, the \a value is the path.
     * See addTranslationCatalog() for more information about translation catalogs.
     *
     * \since Cutelyst 1.5.0
     */
    void addTranslationCatalogs(const QHash<QString, QString> &catalogs);

    /**
     * Loads translations for a specific @p filename from a single directory and returns a list of added locales.
     *
     * This can be used to load translations for a template if the translation file names follow a common schema.
     * Let us assume you organised your translation files as follows:
     * @li @c /usr/share/myapp/translations/mytemplate_de.qm
     * @li @c /usr/share/myapp/translations/mytemplate_pt_BR.qm
     * @li @c ...
     *
     * You can then use loadTranslationsFromDir() on your registered GrantleeView object as follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *      auto view = new GrantleeView(this);
     *      view->loadTranslationsFromDir(QStringLiteral("mytemplate"), QStringLiteral("/usr/share/myapp/translations"), QStringLiteral("_"));
     * }
     * @endcode
     *
     * @p prefix is the part between the file name and the locale part. In the example above it is @c "_", if it is not set the default @c "." will be used. The
     * @p suffix is the file name suffix that defaults to <code>".qm"</code>.
     *
     * @sa addTranslator(), loadTranslationsFromDir()
     *
     * @since Cuteylst 2.1.0
     */
    QVector<QLocale> loadTranslationsFromDir(const QString &filename, const QString &directory, const QString &prefix = QStringLiteral("."), const QString &suffix = QStringLiteral(".qm"));

Q_SIGNALS:
    void changed();
};

}

#endif // GRANTLEE_VIEW_H
