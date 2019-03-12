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
#include "grantleeview_p.h"

#include "application.h"
#include "context.h"
#include "action.h"
#include "response.h"
#include "config.h"

#include <grantlee/qtlocalizer.h>

#include <QString>
#include <QDirIterator>
#include <QtCore/QLoggingCategory>
#include <QTranslator>

Q_LOGGING_CATEGORY(CUTELYST_GRANTLEE, "cutelyst.grantlee", QtWarningMsg)

using namespace Cutelyst;

GrantleeView::GrantleeView(QObject *parent, const QString &name) : View(new GrantleeViewPrivate, parent, name)
{
    Q_D(GrantleeView);

    d->loader = QSharedPointer<Grantlee::FileSystemTemplateLoader>(new Grantlee::FileSystemTemplateLoader);

    d->engine = new Grantlee::Engine(this);
    d->engine->addTemplateLoader(d->loader);
    d->engine->addPluginPath(QStringLiteral(CUTELYST_PLUGINS_DIR));
    d->engine->addDefaultLibrary(QStringLiteral("grantlee_cutelyst"));

    auto app = qobject_cast<Application *>(parent);
    if (app) {
        // make sure templates can be found on the current directory
        setIncludePaths({ app->config(QStringLiteral("root")).toString() });

        // If CUTELYST_VAR is set the template might have become
        // {{ Cutelyst.req.base }} instead of {{ c.req.base }}
        d->cutelystVar = app->config(QStringLiteral("CUTELYST_VAR"), QStringLiteral("c")).toString();

        app->loadTranslations(QStringLiteral("plugin_view_grantlee"));
    } else {
        // make sure templates can be found on the current directory
        setIncludePaths({ QDir::currentPath() });
    }
}

QStringList GrantleeView::includePaths() const
{
    Q_D(const GrantleeView);
    return d->includePaths;
}

void GrantleeView::setIncludePaths(const QStringList &paths)
{
    Q_D(GrantleeView);
    d->loader->setTemplateDirs(paths);
    d->includePaths = paths;
    Q_EMIT changed();
}

QString GrantleeView::templateExtension() const
{
    Q_D(const GrantleeView);
    return d->extension;
}

void GrantleeView::setTemplateExtension(const QString &extension)
{
    Q_D(GrantleeView);
    d->extension = extension;
    Q_EMIT changed();
}

QString GrantleeView::wrapper() const
{
    Q_D(const GrantleeView);
    return d->wrapper;
}

void GrantleeView::setWrapper(const QString &name)
{
    Q_D(GrantleeView);
    d->wrapper = name;
    Q_EMIT changed();
}

void GrantleeView::setCache(bool enable)
{
    Q_D(GrantleeView);

    if (enable != d->cache.isNull()) {
        return; // already enabled
    }

    delete d->engine;
    d->engine = new Grantlee::Engine(this);

    if (enable) {
        d->cache = QSharedPointer<Grantlee::CachingLoaderDecorator>(new Grantlee::CachingLoaderDecorator(d->loader));
        d->engine->addTemplateLoader(d->cache);
    } else {
        d->cache.clear();
        d->engine->addTemplateLoader(d->loader);
    }
    Q_EMIT changed();
}

Grantlee::Engine *GrantleeView::engine() const
{
    Q_D(const GrantleeView);
    return d->engine;
}

void GrantleeView::preloadTemplates()
{
    Q_D(GrantleeView);

    if (!isCaching()) {
        setCache(true);
    }

    const auto includePaths = d->includePaths;
    for (const QString &includePath : includePaths) {
        QDirIterator it(includePath, {
                            QLatin1Char('*') + d->extension
                        },
                        QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            path.remove(includePath);
            if (path.startsWith(QLatin1Char('/'))) {
                path.remove(0, 1);
            }

            if (d->cache->canLoadTemplate(path)) {
                d->cache->loadByName(path, d->engine);
            }
        }
    }
}

bool GrantleeView::isCaching() const
{
    Q_D(const GrantleeView);
    return !d->cache.isNull();
}

QByteArray GrantleeView::render(Context *c) const
{
    Q_D(const GrantleeView);

    QByteArray ret;
    c->setStash(d->cutelystVar, QVariant::fromValue(c));
    const QVariantHash stash = c->stash();
    auto it = stash.constFind(QStringLiteral("template"));
    QString templateFile;
    if (it != stash.constEnd()) {
        templateFile = it.value().toString();
    } else {
        if (c->action() && !c->action()->reverse().isEmpty()) {
            templateFile = c->action()->reverse() + d->extension;
            if (templateFile.startsWith(QLatin1Char('/'))) {
                templateFile.remove(0, 1);
            }
        }

        if (templateFile.isEmpty()) {
            c->error(QStringLiteral("Cannot render template, template name or template stash key not defined"));
            return ret;
        }
    }

    qCDebug(CUTELYST_GRANTLEE) << "Rendering template" << templateFile;

    Grantlee::Context gc(stash);

    auto localizer = QSharedPointer<Grantlee::QtLocalizer>::create(c->locale());

    auto transIt = d->translators.constFind(c->locale());
    if (transIt != d->translators.constEnd()) {
        localizer.data()->installTranslator(transIt.value(), transIt.key().name());
    }

    auto catalogIt = d->translationCatalogs.constBegin();
    while (catalogIt != d->translationCatalogs.constEnd()) {
        localizer.data()->loadCatalog(catalogIt.value(), catalogIt.key());
        ++it;
    }

    gc.setLocalizer(localizer);

    Grantlee::Template tmpl = d->engine->loadByName(templateFile);
    if (tmpl->error() != Grantlee::NoError) {
        c->res()->setBody(c->translate("Cutelyst::GrantleeView", "Internal server error."));
        c->error(QLatin1String("Error while rendering template: ") + tmpl->errorString());
        return ret;
    }

    QString content = tmpl->render(&gc);
    if (tmpl->error() != Grantlee::NoError) {
        c->res()->setBody(c->translate("Cutelyst::GrantleeView", "Internal server error."));
        c->error(QLatin1String("Error while rendering template: ") + tmpl->errorString());
        return ret;
    }

    if (!d->wrapper.isEmpty()) {
        Grantlee::Template wrapper = d->engine->loadByName(d->wrapper);
        if (tmpl->error() != Grantlee::NoError) {
            c->res()->setBody(c->translate("Cutelyst::GrantleeView", "Internal server error."));
            c->error(QLatin1String("Error while rendering template: ") + tmpl->errorString());
            return ret;
        }

        Grantlee::SafeString safeContent(content, true);
        gc.insert(QStringLiteral("content"), safeContent);
        content = wrapper->render(&gc);

        if (wrapper->error() != Grantlee::NoError) {
            c->res()->setBody(c->translate("Cutelyst::GrantleeView", "Internal server error."));
            c->error(QLatin1String("Error while rendering template: ") + tmpl->errorString());
            return ret;
        }
    }

    ret = content.toUtf8();
    return ret;
}

void GrantleeView::addTranslator(const QLocale &locale, QTranslator *translator)
{
    Q_D(GrantleeView);
    Q_ASSERT_X(translator, "add translator to GrantleeView", "invalid QTranslator object");
    d->translators.insert(locale, translator);
}

void GrantleeView::addTranslator(const QString &locale, QTranslator *translator)
{
    addTranslator(QLocale(locale), translator);
}

void GrantleeView::addTranslationCatalog(const QString &path, const QString &catalog)
{
    Q_D(GrantleeView);
    Q_ASSERT_X(!path.isEmpty(), "add translation catalog to GrantleeView", "empty path");
    Q_ASSERT_X(!catalog.isEmpty(), "add translation catalog to GrantleeView", "empty catalog name");
    d->translationCatalogs.insert(catalog, path);
}

void GrantleeView::addTranslationCatalogs(const QHash<QString, QString> &catalogs)
{
    Q_D(GrantleeView);
    Q_ASSERT_X(!catalogs.empty(), "add translation catalogs to GranteleeView", "empty QHash");
    d->translationCatalogs.unite(catalogs);
}

QVector<QLocale> GrantleeView::loadTranslationsFromDir(const QString &filename, const QString &directory, const QString &prefix, const QString &suffix)
{
    QVector<QLocale> locales;

    if (Q_LIKELY(!filename.isEmpty() && !directory.isEmpty())) {
        const QDir i18nDir(directory);
        if (Q_LIKELY(i18nDir.exists())) {
            const QString _prefix = prefix.isEmpty() ? QStringLiteral(".") : prefix;
            const QString _suffix = suffix.isEmpty() ? QStringLiteral(".qm") : suffix;
            const QStringList namesFilter = QStringList({filename + _prefix + QLatin1Char('*') + _suffix});
            const QFileInfoList tsFiles = i18nDir.entryInfoList(namesFilter, QDir::Files);
            if (Q_LIKELY(!tsFiles.empty())) {
                locales.reserve(tsFiles.size());
                for (const QFileInfo &ts : tsFiles) {
                    const QString fn = ts.fileName();
                    const int prefIdx = fn.indexOf(_prefix);
                    const QString locString = fn.mid(prefIdx + _prefix.length(), fn.length() - prefIdx - _suffix.length() - _prefix.length());
                    QLocale loc(locString);
                    if (Q_LIKELY(loc.language() != QLocale::C)) {
                        auto trans = new QTranslator(this);
                        if (Q_LIKELY(trans->load(loc, filename, _prefix, directory))) {
                            addTranslator(loc, trans);
                            locales.append(loc);
                            qCDebug(CUTELYST_GRANTLEE) << "Loaded translations for locale" << loc << "from" << ts.absoluteFilePath();
                        } else {
                            delete trans;
                            qCWarning(CUTELYST_GRANTLEE) << "Can not load translations for locale" << loc;
                        }
                    } else {
                        qCWarning(CUTELYST_GRANTLEE) << "Can not load translations for invalid locale string" << locString;
                    }
                }
                locales.squeeze();
            } else {
                qCWarning(CUTELYST_GRANTLEE) << "Can not find translation files for" << filename << "in directory" << directory;
            }
        } else {
            qCWarning(CUTELYST_GRANTLEE) << "Can not load translations from not existing directory:" << directory;
        }
    } else {
        qCWarning(CUTELYST_GRANTLEE) << "Can not load translations for empty file name or empty path.";
    }

    return locales;
}

#include "moc_grantleeview.cpp"
