/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "application_p.h"
#include "common.h"
#include "config.h"
#include "context_p.h"
#include "controller.h"
#include "controller_p.h"
#include "dispatchtype.h"
#include "enginerequest.h"
#include "request.h"
#include "request_p.h"
#include "response.h"
#include "response_p.h"
#include "stats.h"
#include "utils.h"
#include "view.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDataStream>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QLocale>
#include <QtCore/QPluginLoader>
#include <QtCore/QStringList>
#include <QtCore/QTranslator>

Q_LOGGING_CATEGORY(CUTELYST_DISPATCHER, "cutelyst.dispatcher", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_DISPATCHER_PATH, "cutelyst.dispatcher.path", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_DISPATCHER_CHAINED, "cutelyst.dispatcher.chained", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_CONTROLLER, "cutelyst.controller", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_CORE, "cutelyst.core", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_ENGINE, "cutelyst.engine", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_UPLOAD, "cutelyst.upload", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_MULTIPART, "cutelyst.multipart", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_VIEW, "cutelyst.view", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_REQUEST, "cutelyst.request", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_RESPONSE, "cutelyst.response", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_STATS, "cutelyst.stats", QtWarningMsg)
Q_LOGGING_CATEGORY(CUTELYST_COMPONENT, "cutelyst.component", QtWarningMsg)

using namespace Cutelyst;

Application::Application(QObject *parent)
    : QObject(parent)
    , d_ptr(new ApplicationPrivate)
{
    Q_D(Application);

    d->q_ptr = this;

    qRegisterMetaType<ParamsMultiMap>();
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    qRegisterMetaTypeStreamOperators<ParamsMultiMap>("ParamsMultiMap");
#endif

    d->dispatcher = new Dispatcher(this);

    loadTranslations(QStringLiteral("cutelystcore"));
}

Application::~Application()
{
    delete d_ptr;
}

bool Application::init()
{
    qCDebug(CUTELYST_CORE) << "Default Application::init called on pid:" << QCoreApplication::applicationPid();
    return true;
}

bool Application::postFork()
{
    qCDebug(CUTELYST_CORE) << "Default Application::postFork called on pid:" << QCoreApplication::applicationPid();
    return true;
}

Headers &Application::defaultHeaders() noexcept
{
    Q_D(Application);
    return d->headers;
}

void Application::addXCutelystVersionHeader()
{
    Q_D(Application);
    d->headers.setHeader(QStringLiteral("X_CUTELYST"), QStringLiteral(VERSION));
}

bool Application::registerPlugin(Plugin *plugin)
{
    Q_D(Application);
    if (d->plugins.contains(plugin)) {
        return false;
    }
    d->plugins.append(plugin);
    return true;
}

bool Application::registerController(Controller *controller)
{
    Q_D(Application);
    const auto name = QString::fromLatin1(controller->metaObject()->className());
    if (d->controllersHash.contains(name)) {
        return false;
    }
    d->controllersHash.insert(name, controller);
    d->controllers.append(controller);
    return true;
}

bool Application::registerView(View *view)
{
    Q_D(Application);
    if (d->views.contains(view->name())) {
        qCWarning(CUTELYST_CORE) << "Not registering View." << view->metaObject()->className()
                                 << "There is already a view with this name:" << view->name();
        return false;
    }
    d->views.insert(view->name(), view);
    return true;
}

bool Application::registerDispatcher(DispatchType *dispatcher)
{
    Q_D(Application);
    if (d->dispatchers.contains(dispatcher)) {
        return false;
    }
    d->dispatchers.append(dispatcher);
    return true;
}

Component *Application::createComponentPlugin(const QString &name, QObject *parent)
{
    Q_D(Application);
    auto it = d->factories.constFind(name);
    if (it != d->factories.constEnd()) {
        ComponentFactory *factory = it.value();
        if (factory) {
            return factory->createComponent(parent);
        } else {
            return nullptr;
        }
    }

    const QByteArrayList dirs = QByteArrayList{QByteArrayLiteral(CUTELYST_PLUGINS_DIR)} + qgetenv("CUTELYST_PLUGINS_DIR").split(';');
    for (const QByteArray &dir : dirs) {
        Component *component = d->createComponentPlugin(name, parent, QString::fromLocal8Bit(dir));
        if (component) {
            return component;
        }
    }
    qCDebug(CUTELYST_CORE) << "Did not find plugin" << name << "on" << dirs << "for" << parent;

    return nullptr;
}

const char *Application::cutelystVersion() noexcept
{
    return VERSION;
}

QVector<Cutelyst::Controller *> Application::controllers() const noexcept
{
    Q_D(const Application);
    return d->controllers;
}

View *Application::view(const QString &name) const
{
    Q_D(const Application);
    return d->views.value(name);
}

View *Application::view(QStringView name) const
{
    Q_D(const Application);
    return d->views.value(name);
}

QVariant Application::config(const QString &key, const QVariant &defaultValue) const
{
    Q_D(const Application);
    auto it = d->config.constFind(key);
    if (it != d->config.constEnd()) {
        return it.value();
    }
    return defaultValue;
}

Dispatcher *Application::dispatcher() const noexcept
{
    Q_D(const Application);
    return d->dispatcher;
}

QVector<Cutelyst::DispatchType *> Application::dispatchers() const noexcept
{
    Q_D(const Application);
    return d->dispatcher->dispatchers();
}

QVector<Plugin *> Application::plugins() const noexcept
{
    Q_D(const Application);
    return d->plugins;
}

QVariantMap Application::config() const noexcept
{
    Q_D(const Application);
    return d->config;
}

QString Application::pathTo(const QString &path) const
{
    QDir home = config(QStringLiteral("home")).toString();
    return home.absoluteFilePath(path);
}

QString Cutelyst::Application::pathTo(const QStringList &path) const
{
    QDir home = config(QStringLiteral("home")).toString();
    return home.absoluteFilePath(path.join(u'/'));
}

bool Cutelyst::Application::inited() const noexcept
{
    Q_D(const Application);
    return d->init;
}

Cutelyst::Engine *Cutelyst::Application::engine() const noexcept
{
    Q_D(const Application);
    return d->engine;
}

void Application::setConfig(const QString &key, const QVariant &value)
{
    Q_D(Application);
    d->config.insert(key, value);
}

bool Application::setup(Engine *engine)
{
    Q_D(Application);

    if (d->init) {
        return true;
    }
    d->init = true;

    d->useStats = CUTELYST_STATS().isDebugEnabled();
    d->engine   = engine;
    d->config   = engine->config(QLatin1String("Cutelyst"));

    d->setupHome();

    // Call the virtual application init
    // to setup Controllers plugins stuff
    if (init()) {
        d->setupChildren(children());

        bool zeroCore = engine->workerCore() == 0;

        QVector<QStringList> tablePlugins;
        const auto plugins = d->plugins;
        for (Plugin *plugin : plugins) {
            if (plugin->objectName().isEmpty()) {
                plugin->setObjectName(QString::fromLatin1(plugin->metaObject()->className()));
            }
            tablePlugins.append({plugin->objectName()});
            // Configure plugins
            plugin->setup(this);
        }

        if (zeroCore && !tablePlugins.isEmpty()) {
            qCDebug(CUTELYST_CORE) << Utils::buildTable(tablePlugins, QStringList(), QLatin1String("Loaded plugins:")).constData();
        }

        if (zeroCore) {
            QVector<QStringList> tableDataHandlers;
            tableDataHandlers.append({QLatin1String("application/x-www-form-urlencoded")});
            tableDataHandlers.append({QLatin1String("application/json")});
            tableDataHandlers.append({QLatin1String("multipart/form-data")});
            qCDebug(CUTELYST_CORE) << Utils::buildTable(tableDataHandlers, QStringList(), QLatin1String("Loaded Request Data Handlers:")).constData();

            qCDebug(CUTELYST_CORE) << "Loaded dispatcher" << QString::fromLatin1(d->dispatcher->metaObject()->className());
            qCDebug(CUTELYST_CORE) << "Using engine" << QString::fromLatin1(d->engine->metaObject()->className());
        }

        QString home = d->config.value(QLatin1String("home")).toString();
        if (home.isEmpty()) {
            if (zeroCore) {
                qCDebug(CUTELYST_CORE) << "Couldn't find home";
            }
        } else {
            QFileInfo homeInfo(home);
            if (homeInfo.isDir()) {
                if (zeroCore) {
                    qCDebug(CUTELYST_CORE) << "Found home" << home;
                }
            } else {
                if (zeroCore) {
                    qCDebug(CUTELYST_CORE) << "Home" << home << "doesn't exist";
                }
            }
        }

        QVector<QStringList> table;
        QStringList controllerNames = d->controllersHash.keys();
        controllerNames.sort();
        for (const QString &controller : controllerNames) {
            table.append({controller, QLatin1String("Controller")});
        }

        const auto views = d->views;
        for (View *view : views) {
            if (view->reverse().isEmpty()) {
                const QString className = QString::fromLatin1(view->metaObject()->className()) + QLatin1String("->execute");
                view->setReverse(className);
            }
            table.append({view->reverse(), QLatin1String("View")});
        }

        if (zeroCore && !table.isEmpty()) {
            qCDebug(CUTELYST_CORE) << Utils::buildTable(table, {QLatin1String("Class"), QLatin1String("Type")}, QLatin1String("Loaded components:")).constData();
        }

        const auto controllers = d->controllers;
        for (Controller *controller : controllers) {
            controller->d_ptr->init(this, d->dispatcher);
        }

        d->dispatcher->setupActions(d->controllers, d->dispatchers, d->engine->workerCore() == 0);

        if (zeroCore) {
            qCInfo(CUTELYST_CORE) << qPrintable(QString::fromLatin1("%1 powered by Cutelyst %2, Qt %3.")
                                                    .arg(QCoreApplication::applicationName(),
                                                         QLatin1String(Application::cutelystVersion()),
                                                         QLatin1String(qVersion())));
        }

        Q_EMIT preForked(this);

        return true;
    }

    return false;
}

void Application::handleRequest(EngineRequest *request)
{
    Q_D(Application);

    Engine *engine = d->engine;

    auto priv = new ContextPrivate(this, engine, d->dispatcher, d->plugins);
    auto c    = new Context(priv);

    request->context    = c;
    priv->engineRequest = request;
    priv->response      = new Response(d->headers, request);
    priv->request       = new Request(request);

    if (d->useStats) {
        priv->stats = new Stats(request);
    }

    // Process request
    bool skipMethod = false;
    Q_EMIT beforePrepareAction(c, &skipMethod);

    if (!skipMethod) {
        static bool log = CUTELYST_REQUEST().isEnabled(QtDebugMsg);
        if (log) {
            d->logRequest(priv->request);
        }

        d->dispatcher->prepareAction(c);

        Q_EMIT beforeDispatch(c);

        d->dispatcher->dispatch(c);

        if (request->status & EngineRequest::Async) {
            return;
        }

        Q_EMIT afterDispatch(c);
    }

    c->finalize();
}

bool Application::enginePostFork()
{
    Q_D(Application);

    if (!postFork()) {
        return false;
    }

    const auto controllers = d->controllers;
    for (Controller *controller : controllers) {
        if (!controller->postFork(this)) {
            return false;
        }
    }

    Q_EMIT postForked(this);

    return true;
}

void Application::addTranslator(const QLocale &locale, QTranslator *translator)
{
    Q_D(Application);
    Q_ASSERT_X(translator, "add translator to application", "invalid QTranslator object");
    auto it = d->translators.find(locale);
    if (it != d->translators.end()) {
        it.value().prepend(translator);
    } else {
        d->translators.insert(locale, QVector<QTranslator *>(1, translator));
    }
}

void Application::addTranslator(const QString &locale, QTranslator *translator)
{
    addTranslator(QLocale(locale), translator);
}

void Application::addTranslators(const QLocale &locale, const QVector<QTranslator *> &translators)
{
    Q_D(Application);
    Q_ASSERT_X(!translators.empty(), "add translators to application", "empty translators vector");
    auto transIt = d->translators.find(locale);
    if (transIt != d->translators.end()) {
        for (auto it = translators.crbegin(); it != translators.crend(); ++it) {
            transIt.value().prepend(*it);
        }
    } else {
        d->translators.insert(locale, translators);
    }
}

static void replacePercentN(QString *result, int n)
{
    if (n >= 0) {
        auto percentPos = 0;
        auto len        = 0;
        while ((percentPos = result->indexOf(u'%', percentPos + len)) != -1) {
            len = 1;
            QString fmt;
            if (result->at(percentPos + len) == u'L') {
                ++len;
                fmt = QStringLiteral("%L1");
            } else {
                fmt = QStringLiteral("%1");
            }
            if (result->at(percentPos + len) == u'n') {
                fmt = fmt.arg(n);
                ++len;
                result->replace(percentPos, len, fmt);
                len = fmt.length();
            }
        }
    }
}

QString Application::translate(const QLocale &locale, const char *context, const char *sourceText, const char *disambiguation, int n) const
{
    QString result;

    if (!sourceText) {
        return result;
    }

    Q_D(const Application);

    const QVector<QTranslator *> translators = d->translators.value(locale);
    if (translators.empty()) {
        result = QString::fromUtf8(sourceText);
        replacePercentN(&result, n);
        return result;
    }

    for (QTranslator *translator : translators) {
        result = translator->translate(context, sourceText, disambiguation, n);
        if (!result.isEmpty()) {
            break;
        }
    }

    if (result.isEmpty()) {
        result = QString::fromUtf8(sourceText);
    }

    replacePercentN(&result, n);
    return result;
}

void Application::loadTranslations(const QString &filename, const QString &directory, const QString &prefix, const QString &suffix)
{
    loadTranslationsFromDir(filename, directory, prefix, suffix);
}

QVector<QLocale> Application::loadTranslationsFromDir(const QString &filename, const QString &directory, const QString &prefix, const QString &suffix)
{
    QVector<QLocale> locales;

    if (Q_LIKELY(!filename.isEmpty())) {
        const QString _dir = directory.isEmpty() ? QStringLiteral(I18NDIR) : directory;
        const QDir i18nDir(_dir);
        if (Q_LIKELY(i18nDir.exists())) {
            const QString _prefix         = prefix.isEmpty() ? QStringLiteral(".") : prefix;
            const QString _suffix         = suffix.isEmpty() ? QStringLiteral(".qm") : suffix;
            const QStringList namesFilter = QStringList({filename + _prefix + u'*' + _suffix});

            const QFileInfoList tsFiles = i18nDir.entryInfoList(namesFilter, QDir::Files);
            if (Q_LIKELY(!tsFiles.empty())) {
                locales.reserve(tsFiles.size());
                for (const QFileInfo &ts : tsFiles) {
                    const QString fn        = ts.fileName();
                    const int prefIdx       = fn.indexOf(_prefix);
                    const QString locString = fn.mid(prefIdx + _prefix.length(), fn.length() - prefIdx - _suffix.length() - _prefix.length());
                    QLocale loc(locString);
                    if (Q_LIKELY(loc.language() != QLocale::C)) {
                        auto trans = new QTranslator(this);
                        if (Q_LIKELY(trans->load(loc, filename, _prefix, _dir))) {
                            addTranslator(loc, trans);
                            locales.append(loc);
                            qCDebug(CUTELYST_CORE) << "Loaded translations for" << loc << "from" << ts.absoluteFilePath();
                        } else {
                            delete trans;
                            qCWarning(CUTELYST_CORE) << "Can not load translations for" << loc << "from" << ts.absoluteFilePath();
                        }
                    } else {
                        qCWarning(CUTELYST_CORE) << "Can not load translations for invalid locale string" << locString;
                    }
                }
                locales.squeeze();
            } else {
                qCWarning(CUTELYST_CORE) << "Can not find translation files for" << filename << "in directory" << _dir;
            }
        } else {
            qCWarning(CUTELYST_CORE) << "Can not load translations from not existing directory:" << _dir;
        }
    } else {
        qCWarning(CUTELYST_CORE) << "Can not load translations for empty file name.";
    }

    return locales;
}

QVector<QLocale> Application::loadTranslationsFromDirs(const QString &directory, const QString &filename)
{
    QVector<QLocale> locales;

    if (Q_LIKELY(!directory.isEmpty() && !filename.isEmpty())) {
        const QDir dir(directory);
        if (Q_LIKELY(dir.exists())) {
            const auto dirs = dir.entryList(QDir::AllDirs);
            if (Q_LIKELY(!dirs.empty())) {
                locales.reserve(dirs.size());
                for (const QString &subDir : dirs) {
                    const QString relFn = subDir + u'/' + filename;
                    if (dir.exists(relFn)) {
                        const QLocale l(subDir);
                        if (Q_LIKELY(l.language() != QLocale::C)) {
                            auto trans = new QTranslator(this);
                            const QFileInfo fi(dir, relFn);
                            if (Q_LIKELY(trans->load(l, fi.baseName(), QString(), fi.absolutePath(), fi.suffix()))) {
                                addTranslator(l, trans);
                                locales.append(l);
                                qCDebug(CUTELYST_CORE) << "Loaded translations for" << l << "from" << fi.absoluteFilePath();
                            } else {
                                delete trans;
                                qCWarning(CUTELYST_CORE) << "Can not load translations for" << l << "from" << fi.absoluteFilePath();
                            }
                        } else {
                            qCWarning(CUTELYST_CORE) << "Can not load translations for invalid locale string:" << subDir;
                        }
                    }
                }
                locales.squeeze();
            } else {
                qCWarning(CUTELYST_CORE) << "Can not find locale dirs under" << directory;
            }
        } else {
            qCWarning(CUTELYST_CORE) << "Can not load translations from not existing directory:" << directory;
        }
    } else {
        qCWarning(CUTELYST_CORE) << "Can not load translations for empty file name or directory name";
    }

    return locales;
}

void Cutelyst::ApplicationPrivate::setupHome()
{
    // Hook the current directory in config if "home" is not set
    if (!config.contains(QLatin1String("home"))) {
        config.insert(QStringLiteral("home"), QDir::currentPath());
    }

    if (!config.contains(QLatin1String("root"))) {
        QDir home = config.value(QLatin1String("home")).toString();
        config.insert(QStringLiteral("root"), home.absoluteFilePath(QLatin1String("root")));
    }
}

void ApplicationPrivate::setupChildren(const QObjectList &children)
{
    Q_Q(Application);
    for (QObject *child : children) {
        auto controller = qobject_cast<Controller *>(child);
        if (controller) {
            q->registerController(controller);
            continue;
        }

        auto plugin = qobject_cast<Plugin *>(child);
        if (plugin) {
            q->registerPlugin(plugin);
            continue;
        }

        auto view = qobject_cast<View *>(child);
        if (view) {
            q->registerView(view);
            continue;
        }

        auto dispatchType = qobject_cast<DispatchType *>(child);
        if (dispatchType) {
            q->registerDispatcher(dispatchType);
            continue;
        }
    }
}

void Cutelyst::ApplicationPrivate::logRequest(Request *req)
{
    QString path = req->path();
    if (path.isEmpty()) {
        path = QStringLiteral("/");
    }
    qCDebug(CUTELYST_REQUEST) << req->method() << "request for" << path << "from" << req->addressString();

    ParamsMultiMap params = req->queryParameters();
    if (!params.isEmpty()) {
        logRequestParameters(params, QLatin1String("Query Parameters are:"));
    }

    params = req->bodyParameters();
    if (!params.isEmpty()) {
        logRequestParameters(params, QLatin1String("Body Parameters are:"));
    }

    const auto uploads = req->uploads();
    if (!uploads.isEmpty()) {
        logRequestUploads(uploads);
    }
}

void Cutelyst::ApplicationPrivate::logRequestParameters(const ParamsMultiMap &params, const QString &title)
{
    QVector<QStringList> table;
    auto it = params.constBegin();
    while (it != params.constEnd()) {
        table.append({it.key(), it.value()});
        ++it;
    }
    qCDebug(CUTELYST_REQUEST) << Utils::buildTable(table, {
                                                              QLatin1String("Parameter"),
                                                              QLatin1String("Value"),
                                                          },
                                                   title)
                                     .constData();
}

void Cutelyst::ApplicationPrivate::logRequestUploads(const QVector<Cutelyst::Upload *> &uploads)
{
    QVector<QStringList> table;
    for (Upload *upload : uploads) {
        table.append({upload->name(),
                      upload->filename(),
                      upload->contentType(),
                      QString::number(upload->size())});
    }
    qCDebug(CUTELYST_REQUEST) << Utils::buildTable(table, {
                                                              QLatin1String("Parameter"),
                                                              QLatin1String("Filename"),
                                                              QLatin1String("Type"),
                                                              QLatin1String("Size"),
                                                          },
                                                   QLatin1String("File Uploads are:"))
                                     .constData();
}

Component *ApplicationPrivate::createComponentPlugin(const QString &name, QObject *parent, const QString &directory)
{
    Component *component = nullptr;

    QDir pluginsDir(directory);
    QPluginLoader loader;
    ComponentFactory *factory = nullptr;
    const auto plugins        = pluginsDir.entryList(QDir::Files);
    for (const QString &fileName : plugins) {
        loader.setFileName(pluginsDir.absoluteFilePath(fileName));
        const QJsonObject json = loader.metaData()[QLatin1String("MetaData")].toObject();
        if (json[QLatin1String("name")].toString() == name) {
            QObject *plugin = loader.instance();
            if (plugin) {
                factory = qobject_cast<ComponentFactory *>(plugin);
                if (!factory) {
                    qCCritical(CUTELYST_CORE) << "Could not create a factory for" << loader.fileName();
                } else {
                    component = factory->createComponent(parent);
                }
                break;
            } else {
                qCCritical(CUTELYST_CORE) << "Could not load plugin" << loader.fileName() << loader.errorString();
            }
        }
    }

    if (factory) {
        factories.insert(name, factory);
    }

    return component;
}

#include "moc_application.cpp"
