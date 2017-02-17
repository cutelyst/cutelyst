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

#ifndef GRANTLEE_VIEW_H
#define GRANTLEE_VIEW_H

#include <QObject>
#include <QStringList>
#include <QLocale>

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
 */
class CUTELYST_VIEW_GRANTLEE_EXPORT GrantleeView : public View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GrantleeView)
public:
    explicit GrantleeView(QObject *parent = nullptr, const QString &name = QString());
    ~GrantleeView();

    Q_PROPERTY(QStringList includePaths READ includePaths WRITE setIncludePaths)
    QStringList includePaths() const;
    void setIncludePaths(const QStringList &paths);

    Q_PROPERTY(QString templateExtension READ templateExtension WRITE setTemplateExtension)
    QString templateExtension() const;
    void setTemplateExtension(const QString &extension);

    Q_PROPERTY(QString wrapper READ wrapper WRITE setWrapper)
    QString wrapper() const;
    void setWrapper(const QString &name);

    Q_PROPERTY(bool cache READ isCaching WRITE setCache)
    bool isCaching() const;
    void setCache(bool enable);

    Grantlee::Engine *engine() const;

    void preloadTemplates();

    QByteArray render(Context *c) const final;

    /*!
     * \brief Adds a \a translator for the specified \a locale to the list of translators.
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
     * \since Cutelyst 1.5.0
     */
    void addTranslator(const QLocale &locale, QTranslator *translator);

    /*!
     * \brief Adds a \a translator for the specified \a locale to the list of translators.
     *
     * The \a locale string should be parseable by QLocale.
     *
     * \overload
     *
     * \since Cutelyst 1.4.0
     */
    void addTranslator(const QString &locale, QTranslator *translator);

    /*!
     * \brief Dynamically adds translation \a catalog at \a path to the translator.
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

    /*!
     * \brief Adds a dictionary of translation catalogs and paths to the translator.
     *
     * The \a key of the QHash is the name of the catalog, the \a value is the path.
     * See addTranslationCatalog() for more information about translation catalogs.
     *
     * \since Cutelyst 1.5.0
     */
    void addTranslationCatalogs(const QHash<QString, QString> &catalogs);

protected:
    GrantleeViewPrivate *d_ptr;
};

}

#endif // GRANTLEE_VIEW_H
