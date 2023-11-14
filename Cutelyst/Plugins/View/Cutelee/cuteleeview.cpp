/*
 * SPDX-FileCopyrightText: (C) 2020-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "action.h"
#include "application.h"
#include "config.h"
#include "context.h"
#include "cuteleeview_p.h"
#include "cutelystcutelee.h"
#include "response.h"

#include <cutelee/metatype.h>
#include <cutelee/qtlocalizer.h>

#include <QDirIterator>
#include <QString>
#include <QTranslator>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_CUTELEE, "cutelyst.cutelee", QtWarningMsg)

using namespace Cutelyst;

CUTELEE_BEGIN_LOOKUP(ParamsMultiMap)
return object.value(property);
CUTELEE_END_LOOKUP

CUTELEE_BEGIN_LOOKUP_PTR(Cutelyst::Request)
return object->property(property.toLatin1().constData());
CUTELEE_END_LOOKUP

CuteleeView::CuteleeView(QObject *parent, const QString &name)
    : View(new CuteleeViewPrivate, parent, name)
{
    Q_D(CuteleeView);

    Cutelee::registerMetaType<ParamsMultiMap>();
    Cutelee::registerMetaType<Cutelyst::Request *>(); // To be able to access it's properties

    d->loader = std::make_shared<Cutelee::FileSystemTemplateLoader>();

    d->engine = new Cutelee::Engine(this);
    d->engine->addTemplateLoader(d->loader);

    d->initEngine();

    auto app = qobject_cast<Application *>(parent);
    if (app) {
        // make sure templates can be found on the current directory
        setIncludePaths({app->config(QStringLiteral("root")).toString()});

        // If CUTELYST_VAR is set the template might have become
        // {{ Cutelyst.req.base }} instead of {{ c.req.base }}
        d->cutelystVar =
            app->config(QStringLiteral("CUTELYST_VAR"), QStringLiteral("c")).toString();

        app->loadTranslations(QStringLiteral("plugin_view_cutelee"));
    } else {
        // make sure templates can be found on the current directory
        setIncludePaths({QDir::currentPath()});
    }
}

QStringList CuteleeView::includePaths() const
{
    Q_D(const CuteleeView);
    return d->includePaths;
}

void CuteleeView::setIncludePaths(const QStringList &paths)
{
    Q_D(CuteleeView);
    d->loader->setTemplateDirs(paths);
    d->includePaths = paths;
    Q_EMIT changed();
}

QString CuteleeView::templateExtension() const
{
    Q_D(const CuteleeView);
    return d->extension;
}

void CuteleeView::setTemplateExtension(const QString &extension)
{
    Q_D(CuteleeView);
    d->extension = extension;
    Q_EMIT changed();
}

QString CuteleeView::wrapper() const
{
    Q_D(const CuteleeView);
    return d->wrapper;
}

void CuteleeView::setWrapper(const QString &name)
{
    Q_D(CuteleeView);
    d->wrapper = name;
    Q_EMIT changed();
}

void CuteleeView::setCache(bool enable)
{
    Q_D(CuteleeView);

    if (enable && d->cache) {
        return; // already enabled
    }

    delete d->engine;
    d->engine = new Cutelee::Engine(this);

    if (enable) {
        d->cache = std::make_shared<Cutelee::CachingLoaderDecorator>(d->loader);
        d->engine->addTemplateLoader(d->cache);
    } else {
        d->cache = {};
        d->engine->addTemplateLoader(d->loader);
    }
    d->initEngine();
    Q_EMIT changed();
}

Cutelee::Engine *CuteleeView::engine() const
{
    Q_D(const CuteleeView);
    return d->engine;
}

void CuteleeView::preloadTemplates()
{
    Q_D(CuteleeView);

    if (!isCaching()) {
        setCache(true);
    }

    const auto includePaths = d->includePaths;
    for (const QString &includePath : includePaths) {
        QDirIterator it(includePath,
                        {QLatin1Char('*') + d->extension},
                        QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            QString path = it.next();
            path.remove(includePath);
            if (path.startsWith(u'/')) {
                path.remove(0, 1);
            }

            if (d->cache->canLoadTemplate(path)) {
                d->cache->loadByName(path, d->engine);
            }
        }
    }
}

bool CuteleeView::isCaching() const
{
    Q_D(const CuteleeView);
    return !!d->cache;
}

QByteArray CuteleeView::render(Context *c) const
{
    Q_D(const CuteleeView);

    QByteArray ret;
    c->setStash(d->cutelystVar, QVariant::fromValue(c));
    const QVariantHash stash = c->stash();
    auto it                  = stash.constFind(QStringLiteral("template"));
    QString templateFile;
    if (it != stash.constEnd()) {
        templateFile = it.value().toString();
    } else {
        if (c->action() && !c->action()->reverse().isEmpty()) {
            templateFile = c->action()->reverse() + d->extension;
            if (templateFile.startsWith(u'/')) {
                templateFile.remove(0, 1);
            }
        }

        if (templateFile.isEmpty()) {
            c->appendError(QStringLiteral(
                "Cannot render template, template name or template stash key not defined"));
            return ret;
        }
    }

    qCDebug(CUTELYST_CUTELEE) << "Rendering template" << templateFile;

    Cutelee::Context gc(stash);

    auto localizer = std::make_shared<Cutelee::QtLocalizer>(c->locale());

    auto transIt = d->translators.constFind(c->locale());
    if (transIt != d->translators.constEnd()) {
        localizer.get()->installTranslator(transIt.value(), transIt.key().name());
    }

    auto catalogIt = d->translationCatalogs.constBegin();
    while (catalogIt != d->translationCatalogs.constEnd()) {
        localizer.get()->loadCatalog(catalogIt.value(), catalogIt.key());
        ++it;
    }

    gc.setLocalizer(localizer);

    Cutelee::Template tmpl = d->engine->loadByName(templateFile);
    if (tmpl->error() != Cutelee::NoError) {
        //% "Internal server error."
        c->res()->setBody(c->qtTrId("cutelyst-cuteleeview-err-internal-server"));
        c->appendError(QLatin1String("Error while rendering template: ") + tmpl->errorString());
        return ret;
    }

    QString content = tmpl->render(&gc);
    if (tmpl->error() != Cutelee::NoError) {
        c->res()->setBody(c->qtTrId("cutelyst-cuteleeview-err-internal-server"));
        c->appendError(QLatin1String("Error while rendering template: ") + tmpl->errorString());
        return ret;
    }

    if (!d->wrapper.isEmpty()) {
        Cutelee::Template wrapper = d->engine->loadByName(d->wrapper);
        if (tmpl->error() != Cutelee::NoError) {
            c->res()->setBody(c->qtTrId("cutelyst-cuteleeview-err-internal-server"));
            c->appendError(QLatin1String("Error while rendering template: ") + tmpl->errorString());
            return ret;
        }

        Cutelee::SafeString safeContent(content, true);
        gc.insert(QStringLiteral("content"), safeContent);
        content = wrapper->render(&gc);

        if (wrapper->error() != Cutelee::NoError) {
            c->res()->setBody(c->qtTrId("cutelyst-cuteleeview-err-internal-server"));
            c->appendError(QLatin1String("Error while rendering template: ") + tmpl->errorString());
            return ret;
        }
    }

    ret = content.toUtf8();
    return ret;
}

void CuteleeView::addTranslator(const QLocale &locale, QTranslator *translator)
{
    Q_D(CuteleeView);
    Q_ASSERT_X(translator, "add translator to CuteleeView", "invalid QTranslator object");
    d->translators.insert(locale, translator);
}

void CuteleeView::addTranslator(const QString &locale, QTranslator *translator)
{
    addTranslator(QLocale(locale), translator);
}

void CuteleeView::addTranslationCatalog(const QString &path, const QString &catalog)
{
    Q_D(CuteleeView);
    Q_ASSERT_X(!path.isEmpty(), "add translation catalog to CuteleeView", "empty path");
    Q_ASSERT_X(!catalog.isEmpty(), "add translation catalog to CuteleeView", "empty catalog name");
    d->translationCatalogs.insert(catalog, path);
}

void CuteleeView::addTranslationCatalogs(const QMultiHash<QString, QString> &catalogs)
{
    Q_D(CuteleeView);
    Q_ASSERT_X(!catalogs.empty(), "add translation catalogs to GranteleeView", "empty QHash");
    d->translationCatalogs.unite(catalogs);
}

QVector<QLocale> CuteleeView::loadTranslationsFromDir(const QString &filename,
                                                      const QString &directory,
                                                      const QString &prefix,
                                                      const QString &suffix)
{
    QVector<QLocale> locales;

    if (Q_LIKELY(!filename.isEmpty() && !directory.isEmpty())) {
        const QDir i18nDir(directory);
        if (Q_LIKELY(i18nDir.exists())) {
            const QString _prefix = prefix.isEmpty() ? QStringLiteral(".") : prefix;
            const QString _suffix = suffix.isEmpty() ? QStringLiteral(".qm") : suffix;
            const QStringList namesFilter =
                QStringList({filename + _prefix + QLatin1Char('*') + _suffix});
            const QFileInfoList tsFiles = i18nDir.entryInfoList(namesFilter, QDir::Files);
            if (Q_LIKELY(!tsFiles.empty())) {
                locales.reserve(tsFiles.size());
                for (const QFileInfo &ts : tsFiles) {
                    const QString fn  = ts.fileName();
                    const int prefIdx = fn.indexOf(_prefix);
                    const QString locString =
                        fn.mid(prefIdx + _prefix.length(),
                               fn.length() - prefIdx - _suffix.length() - _prefix.length());
                    QLocale loc(locString);
                    if (Q_LIKELY(loc.language() != QLocale::C)) {
                        auto trans = new QTranslator(this);
                        if (Q_LIKELY(trans->load(loc, filename, _prefix, directory))) {
                            addTranslator(loc, trans);
                            locales.append(loc);
                            qCDebug(CUTELYST_CUTELEE) << "Loaded translations for locale" << loc
                                                      << "from" << ts.absoluteFilePath();
                        } else {
                            delete trans;
                            qCWarning(CUTELYST_CUTELEE)
                                << "Can not load translations for locale" << loc;
                        }
                    } else {
                        qCWarning(CUTELYST_CUTELEE)
                            << "Can not load translations for invalid locale string" << locString;
                    }
                }
                locales.squeeze();
            } else {
                qCWarning(CUTELYST_CUTELEE) << "Can not find translation files for" << filename
                                            << "in directory" << directory;
            }
        } else {
            qCWarning(CUTELYST_CUTELEE)
                << "Can not load translations from not existing directory:" << directory;
        }
    } else {
        qCWarning(CUTELYST_CUTELEE)
            << "Can not load translations for empty file name or empty path.";
    }

    return locales;
}

void CuteleeViewPrivate::initEngine()
{
    // Set also the paths from CUTELYST_PLUGINS_DIR env variable as plugin paths of cutelee engine
    const QByteArrayList dirs = QByteArrayList{QByteArrayLiteral(CUTELYST_PLUGINS_DIR)} +
                                qgetenv("CUTELYST_PLUGINS_DIR").split(';');
    for (const QByteArray &dir : dirs) {
        engine->addPluginPath(QString::fromLocal8Bit(dir));
    }

    engine->insertDefaultLibrary(QStringLiteral("cutelee_cutelyst"), new CutelystCutelee(engine));
}

#include "moc_cuteleeview.cpp"
