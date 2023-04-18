/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_APPLICATION_H
#define CUTELYST_APPLICATION_H

#include <Cutelyst/cutelyst_global.h>

#include <QtCore/QLocale>
#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QVector>

class QTranslator;

namespace Cutelyst {

#define CUTELYST_APPLICATION(x) \
    Q_PLUGIN_METADATA(x) \
    Q_INTERFACES(Cutelyst::Application)

class Context;
class Controller;
class Component;
class View;
class Dispatcher;
class DispatchType;
class Request;
class Response;
class Engine;
class EngineRequest;
class Plugin;
class Headers;
class ApplicationPrivate;

/*! \class Application application.h Cutelyst/Application
 * @brief The %Cutelyst %Application
 *
 * This is the main class of a Cutelyst appplication
 */
class CUTELYST_LIBRARY Application : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Application)
public:
    /**
     * The constructor is used to setup the class configuration,
     * subclasses should only use this for objects that do
     * not require configuration to be ready.
     *
     * A Web Engine will instantiate your application through this
     * class, next it will load the settings file, and in the end
     * it will call init() which is where your application
     * should do it's own setup.
     *
     * \warning DO NOT register your controllers,
     * plugins or anything that might want to
     * use config() in here, do that in init()
     */
    explicit Application(QObject *parent = nullptr);
    virtual ~Application();

    /**
     * Returns a list with all registered controllers.
     *
     * The list might only be complete after application has been setup.
     */
    QVector<Controller *> controllers() const noexcept;

    /**
     * Returns the view specified by \p name, if no view is found nullptr is returned.
     */
    View *view(const QString &name) const;

    /**
     * Returns the view specified by \p name, if no view is found nullptr is returned.
     */
    View *view(QStringView name = {}) const;

    /**
     * Returns application config specified by \p key, with a possible default value.
     */
    QVariant config(const QString &key, const QVariant &defaultValue = {}) const;

    /**
     * Returns the dispatcher class.
     */
    Dispatcher *dispatcher() const noexcept;

    /**
     * Returns a list with all registered dispachers.
     *
     * The list might only be complete after application has been setup.
     */
    QVector<DispatchType *> dispatchers() const noexcept;

    /**
     * Returns all registered plugins
     */
    QVector<Plugin *> plugins() const noexcept;

    /*!
     * Returns the registered plugin that casts to the template type \p T
     */
    template <typename T>
    T plugin()
    {
        const auto pluginsConst = plugins();
        for (Plugin *plugin : pluginsConst) {
            auto p = qobject_cast<T>(plugin);
            if (p) {
                return p;
            }
        }
        return nullptr;
    }

    /**
     * User configuration for the application
     * @return A variant hash with configuration settings
     */
    QVariantMap config() const noexcept;

    /**
     * Merges path with config("HOME") and returns an absolute path.
     */
    QString pathTo(const QString &path) const;

    /**
     * Merges path with config("HOME") and returns an absolute path.
     */
    QString pathTo(const QStringList &path) const;

    /**
     * Returns true if the application has been inited.
     */
    bool inited() const noexcept;

    /**
     * Returns current engine that is generating requests.
     */
    Engine *engine() const noexcept;

    /**
     * Tries to load a plugin in Cutelyst default plugin directory with \p parent as it's parent.
     * A nullptr is returned in case of failure.
     */
    Component *createComponentPlugin(const QString &name, QObject *parent = nullptr);

    /**
     * Returns cutelyst version.
     */
    static const char *cutelystVersion() noexcept;

    /**
     * Adds a @a translator for the specified @a locale.
     *
     * You can add multiple translators for different application parts for every supported
     * locale. The installed translators will then be used by Context::translate() (what itself
     * will use Application::translate()) to translate strings according to the locale set by
     * Context::setLocale().
     *
     * @par Usage example:
     * @code{.cpp}
     * bool MyCutelystApp::init()
     * {
     *      // ...
     *
     *      auto trans = new QTranslator(this);
     *      QLocale deDE(QLocale::German, QLocale::Germany);
     *      if (trans->load(deDE, QStringLiteral("mycutelystapp"), QStringLiteral("."), QStringLiteral("/usr/share/mycutelystapp/l10n")) {
     *          addTranslator(deDE, trans);
     *      }
     *
     *      // ...
     * }
     * @endcode
     *
     * @sa loadTranslations()
     *
     * @since Cutelyst 1.5.0
     */
    void addTranslator(const QLocale &locale, QTranslator *translator);

    /**
     * Adds a @a translator for the specified @a locale.
     *
     * The @a locale string has to be parseable by QLocale.
     *
     * @overload
     *
     * @since Cutelyst 1.5.0
     */
    void addTranslator(const QString &locale, QTranslator *translator);

    /**
     * Adds multiple @a translators for the specified @a locale.
     *
     * @sa addTranslator()
     *
     * @since Cutelyst 1.5.0
     */
    void addTranslators(const QLocale &locale, const QVector<QTranslator *> &translators);

    /**
     * Translates the @a sourceText into the target @a locale language.
     *
     * This uses the installed translators for the specified @a locale to translate the @a sourceText for the
     * given @a context into the target locale. Optionally you can use a @a disambiguation and/or the @a n parameter
     * to translate a pluralized version.
     *
     * @sa Context::translate(), QTranslator::translate()
     *
     * @since Cutelyst 1.5.0
     */
    QString translate(const QLocale &locale, const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const;

    /**
     * Loads translations for a specific @a filename from a single directory.
     *
     * This can be used to load translations for a specific component if the translation file names follow a common schema.
     * Let us assume you organised your translation files as follows:
     * @li @c /usr/share/myapp/translations/myapp_de.qm
     * @li @c /usr/share/myapp/translations/myapp_pt_BR.qm
     * @li @c ...
     *
     * You can then use loadTranslations() in your reimplementation of Application::init() as follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *      loadTranslations(QStringLiteral("myapp"), QStringLiteral("/usr/share/myapp/translations"), QStringLiteral("_"));
     * }
     * @endcode
     *
     * If @a directory is empty, the default directory, set by <code>-DI18NDIR</code>, will be used. @a prefix is the part between
     * the file name and the locale part. In the example above it is @c "_", if it is not set the default @c "." will be used. The
     * @a suffix is the file name suffix that defaults to <code>".qm"</code>.
     *
     * @sa addTranslator(), loadTranslationsFromDir(), loadTranslationsFromDirs()
     *
     * @since Cuteylst 2.0.0
     */
    void loadTranslations(const QString &filename, const QString &directory = QString(), const QString &prefix = QString(), const QString &suffix = QString());

    /**
     * Loads translations for a specific @a filename from a single directory and returns a list of added locales.
     *
     * This can be used to load translations for a specific component if the translation file names follow a common schema.
     * Let us assume you organised your translation files as follows:
     * @li @c /usr/share/myapp/translations/myapp_de.qm
     * @li @c /usr/share/myapp/translations/myapp_pt_BR.qm
     * @li @c ...
     *
     * You can then use loadTranslationsFromDir() in your reimplementation of Application::init() as follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *      loadTranslationsFromDir(QStringLiteral("myapp"), QStringLiteral("/usr/share/myapp/translations"), QStringLiteral("_"));
     * }
     * @endcode
     *
     * If @a directory is empty, the default directory, set by <code>-DI18NDIR</code>, will be used. @a prefix is the part between
     * the file name and the locale part. In the example above it is @c "_", if it is not set the default @c "." will be used. The
     * @a suffix is the file name suffix that defaults to <code>".qm"</code>.
     *
     * @sa addTranslator(), loadTranslationsFromDirs()
     *
     * @since Cuteylst 2.1.0
     */
    QVector<QLocale> loadTranslationsFromDir(const QString &filename, const QString &directory = QString(), const QString &prefix = QStringLiteral("."), const QString &suffix = QStringLiteral(".qm"));

    /**
     * Loads translations for a specific @a filename from a directory structure under @a directory and returns a list of added locales.
     *
     * This can be used to load translations for a specific component or application if the the translation files are organized in
     * subdirectories named after locale codes. Let us assume you organised your translation files as follows:
     * @li @c /usr/share/locale/de/LC_MESSAGES/myapp.qm
     * @li @c /usr/share/locale/pt_BR/LC_MESSAGES/myapp.qm
     * @li @c ...
     *
     * You can then use loadTranslationsFromDirs() in your reimplementation of Application::init() as follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *     loadTranslationsFromDirs(QStringLiteral("/usr/share/locale"), QStringLiteral("LC_MESSAGES/myapp.qm"));
     * }
     * @endcode
     *
     * @sa addTranslator(), loadTranslationsFromDir()
     *
     * @since Cutelyst 2.1.0
     */
    QVector<QLocale> loadTranslationsFromDirs(const QString &directory, const QString &filename);

protected:
    /**
     * Do your application initialization here, if your
     * application should not proceed log some information
     * that might help on debuggin and return false
     *
     * For example if your application only works with
     * PostgeSQL and the Qt driver is not available it
     * makes sense to fail here. However you should not
     * initialize resouces that cannot be shared among
     * process. \sa postFork
     *
     * @return \c true if your application was successfuly initted
     */
    virtual bool init();

    /**
     * This method is called after the engine forks
     *
     * After the web engine forks itself it will call
     * this function so that you can initialize resources
     * that can't be shared with the parent process, namely
     * sockets and file descriptors.
     *
     * A good example of usage of this function is when
     * openning a connection to the database which can't
     * be shared with other process and should probably
     * make this function return false if it fails to open.
     *
     * Default implementation returns true.
     *
     * @return False if the engine should not use this process
     */
    virtual bool postFork();

    /**
     * This is the HTTP default response headers that each request gets
     *
     * Do not change it after the application has started.
     */
    Headers &defaultHeaders() noexcept;

    /**
     * Adds a X-Cutelyst header with our version on each request
     */
    void addXCutelystVersionHeader();

    /**
     * Registers a global plugin ie one that doesn't need
     * to be created explicity for a single request and returns
     * true on plugin->isApplicationPlugin();
     *
     * @return \c true if the plugin could be registered
     */
    bool registerPlugin(Plugin *plugin);

    /**
     * This method registers a Controller class which
     * is responsible for handlying Requests,
     * since they are reused between multiple requests
     * beaware of not storing data there, instead you
     * might want to use a session plugin or the stash.
     *
     * @param controller the Controller class
     * @return \c true if succeeded
     */
    bool registerController(Controller *controller);

    /**
     * This method registers a View class which
     * is responsible for rendering requests.
     *
     * @param view the View class
     * @return \c true if succeeded
     */
    bool registerView(View *view);

    /**
     * Registers a custom DispatchType, if none is registered
     * all the built-in dispatchers types will be registered
     */
    bool registerDispatcher(DispatchType *dispatcher);

Q_SIGNALS:
    /**
     * This signal is emitted before the Dispatcher
     * is called to find an action.
     * It's useful if you need to intercept requests
     * before they are dispached.
     * Always check skipMethod and return if it's true.
     * In case you want to stop further processing set
     * skipMethod to true.
     */
    void beforePrepareAction(Cutelyst::Context *c, bool *skipMethod);

    /**
     * This signal is emitted right after the Dispatcher
     * returns the Action that will be executed.
     */
    void beforeDispatch(Cutelyst::Context *c);

    /**
     * This signal is emitted right after the Action
     * found by the dispatcher got executed.
     */
    void afterDispatch(Cutelyst::Context *c);

    /**
     * This signal is emitted right after application has been setup
     * and before application forks and \sa postFork() is called.
     */
    void preForked(Cutelyst::Application *app);

    /**
     * This signal is emitted after before \sa postFork() is called.
     */
    void postForked(Cutelyst::Application *app);

    /**
     * This signal is likely to be emitted when the worker process should
     * stop, at this point the application has a limited time to finish it's
     * operations, if a timeout is reached the application will get killed.
     */
    void shuttingDown(Cutelyst::Application *app);

protected:
    /**
     * Change the value of the configuration key
     * You should never call this from random parts of the
     * code as a way to store shareable data, it should
     * only be called by a subclass
     */
    void setConfig(const QString &key, const QVariant &value);

    friend class Engine;
    friend class Context;

    /*!
     * Called by the Engine to setup the internal data
     */
    bool setup(Engine *engine);

    /*!
     * Called by the Engine to handle a new Request object
     */
    void handleRequest(Cutelyst::EngineRequest *request);

    /*!
     * Called by the Engine once post fork happened
     */
    bool enginePostFork();

    ApplicationPrivate *d_ptr;
};

} // namespace Cutelyst

#define CutelystApplicationInterface_iid "org.cutelyst.CutelystApplicationInterface"

Q_DECLARE_INTERFACE(Cutelyst::Application, CutelystApplicationInterface_iid)

#endif // CUTELYST_APPLICATION_H
