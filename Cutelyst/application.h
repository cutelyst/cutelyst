/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_APPLICATION_H
#define CUTELYST_APPLICATION_H

#include <Cutelyst/cutelyst_export.h>

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

/**
 * \ingroup core
 * \class Application application.h Cutelyst/Application
 * \brief The %Cutelyst application.
 *
 * This is the main class of a %Cutelyst appplication.
 *
 * <h3 id="configfile">Configuration file options</h3>
 *
 * There are some options you can set in your \ref configuration "application configuration file"
 * in the \c %Cutelyst section. If you want to get keys via the config() method of %Application,
 * you have to add them to the \c %Cutelyst section. If you want to get other config sections
 * than \c %Cutelyst, you should use \link Engine::config() engine()->config("section")\endlink.
 *
 * @configblock{home,string,empty}
 * Absolute path to your home diretory, that is for example used by pathTo(). If this is empty
 * (the default), it will be populated by the return value of QDir::currentPath().
 * @endconfigblock
 *
 * @configblock{root,string,empty}
 * Absolute path to your web root directory that contains your template and/or static files. This
 * might be used by plugins to \ref servestatic "serve static files" or by
 * \ref plugins-view "Views" like CuteleeView to find template files. If this is empty (the
 * default), it will be populated as directory \c "root" below \c "home".
 * @endconfigblock
 *
 * \logcat{core}
 */
class CUTELYST_EXPORT Application : public QObject
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
     * should do it’s own setup.
     *
     * \warning DO NOT register your controllers,
     * plugins or anything that might want to
     * use config() in here, do that in init()
     */
    explicit Application(QObject *parent = nullptr);

    /**
     * Destroys the %Application object.
     */
    virtual ~Application();

    /**
     * Returns a list with all registered controllers.
     *
     * The list might only be complete after application has been setup.
     */
    QVector<Controller *> controllers() const noexcept;

    /**
     * Returns the \ref plugins-view specified by \a name, if no view is found nullptr is returned.
     * The default view is nameless.
     */
    View *view(QStringView name = {}) const;

    /**
     * Returns application config specified by \a key. If \a key is not found, \a defaultValue
     * will be returned. These are the config entries in the \c %Cutelyst section in your
     * \ref configuration "application configuration file".
     *
     * \note If you want to get config entries for other sections than \c %Cutelyst, you should
     * use \link Engine::config() engine()->config("section").value("key")\endlink.
     *
     * \sa \ref configuration
     */
    QVariant config(const QString &key, const QVariant &defaultValue = {}) const;

    /**
     * Returns the dispatcher class.
     */
    Dispatcher *dispatcher() const noexcept;

    /**
     * Returns a list with all registered dispatchers.
     *
     * The list might only be complete after application has been setup.
     */
    QVector<DispatchType *> dispatchers() const noexcept;

    /**
     * Returns all registered plugins.
     */
    QVector<Plugin *> plugins() const noexcept;

    /**
     * Returns the registered plugin that casts to the template type \a T.
     */
    template <typename T>
    T plugin()
    {
        const auto pluginsConst = plugins();
        for (Plugin *pluginPtr : pluginsConst) {
            auto p = qobject_cast<T>(pluginPtr);
            if (p) {
                return p;
            }
        }
        return nullptr;
    }

    /**
     * User configuration for the application. Returns a map with
     * configuration settings that are read from the \c %Cutelyst section of your
     * \ref configuration "application configuration file".
     *
     * \note If you want to get other config sections than \c %Cutelyst, you should
     * use \link Engine::config() engine()->config("section")\endlink.
     *
     * \sa \ref configuration
     */
    QVariantMap config() const noexcept;

    /**
     * Merges \a path with \link config() config("home")\endlink and returns an absolute path.
     *
     * \note If "home" was not set in your \ref configuration "configuration file",
     * it will be automatically set to the value returned by QDir::currentPath().
     */
    QString pathTo(const QString &path) const;

    /**
     * Merges the \a path parts with \link config() config("home")\endlink and returns an
     * absolute path.
     *
     * \note If "home" was not set in your \ref configuration "configuration file",
     * it will be automatically set to the value returned by QDir::currentPath().
     */
    QString pathTo(const QStringList &path) const;

    /**
     * Returns \c true if the application has been inited.
     */
    bool inited() const noexcept;

    /**
     * Returns current engine that is generating requests.
     */
    Engine *engine() const noexcept;

    /**
     * Tries to load a plugin in %Cutelyst default plugin directory with \a parent as it’s parent.
     * A nullptr is returned in case of failure.
     */
    Component *createComponentPlugin(const QString &name, QObject *parent = nullptr);

    /**
     * Returns %Cutelyst version.
     */
    static const char *cutelystVersion() noexcept;

    /**
     * Adds a @a translator for the specified @a locale.
     *
     * You can add multiple translators for different application parts for every supported
     * locale. The installed translators will then be used by Context::translate() and
     * Context::qtTrId() (which itself will use Application::translate()) to translate strings
     * according to the locale set by Context::setLocale().
     *
     * \note You will most likely want to use loadTranslationsFromDir() or
     * loadTranslationsFromDirs() instead of this function because they automatically load
     * translations and set their locales.
     *
     * @par Usage example:
     * @code{.cpp}
     * bool MyCutelystApp::init()
     * {
     *      // ...
     *
     *      auto trans = new QTranslator(this);
     *      QLocale deDE(QLocale::German, QLocale::Germany);
     *      if (trans->load(deDE, QStringLiteral("mycutelystapp"),
     *                            QStringLiteral("."),
     *                            QStringLiteral("/usr/share/mycutelystapp/l10n")) {
     *           addTranslator(deDE, trans);
     *      }
     *
     *      // ...
     * }
     * @endcode
     *
     * @sa loadTranslationsFromDir() loadTranslationsFromDirs()
     * @sa @ref translations
     *
     * @since %Cutelyst 1.5.0
     */
    void addTranslator(const QLocale &locale, QTranslator *translator);

    /**
     * Adds a @a translator for the specified @a locale.
     *
     * The @a locale string has to be parseable by QLocale.
     *
     * @overload
     *
     * @sa @ref translations
     *
     * @since %Cutelyst 1.5.0
     */
    void addTranslator(const QString &locale, QTranslator *translator);

    /**
     * Adds multiple @a translators for the specified @a locale.
     *
     * \note You will most likely want to use loadTranslationsFromDir() or
     * loadTranslationsFromDirs() instead of this function because they automatically load
     * translations and set their locales.
     *
     * @sa addTranslator()
     * @sa @ref translations
     *
     * @since %Cutelyst 1.5.0
     */
    void addTranslators(const QLocale &locale, const QVector<QTranslator *> &translators);

    /**
     * Translates the @a sourceText into the target @a locale language.
     *
     * This uses the installed translators for the specified @a locale to translate the @a
     * sourceText for the given @a context into the target locale. Optionally you can use a @a
     * disambiguation and/or the @a n parameter to translate a pluralized version.
     *
     * Do not use this method directly but use Context::translate() or Context::qtTrId().
     *
     * @sa Context::translate(), Context::qtTrId(), QTranslator::translate()
     * @sa @ref translations
     *
     * @since %Cutelyst 1.5.0
     */
    QString translate(const QLocale &locale,
                      const char *context,
                      const char *sourceText,
                      const char *disambiguation = nullptr,
                      int n                      = -1) const;

    /**
     * Loads translations for a specific @a filename from a single directory.
     *
     * This can be used to load translations for a specific component or application if the
     * translation file names follow a common schema.
     *
     * Let us assume you organised your translation files as follows:
     * @li @c /usr/share/myapp/translations/myapp_de.qm
     * @li @c /usr/share/myapp/translations/myapp_pt_BR.qm
     * @li @c ...
     *
     * You can then use loadTranslations() in your reimplementation of Application::init() as
     * follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *      loadTranslations(QStringLiteral("myapp"),
     *                       QStringLiteral("/usr/share/myapp/translations"),
     *                       QStringLiteral("_"));
     * }
     * @endcode
     *
     * If @a directory is empty, the default directory, set by <code>-DI18NDIR</code>, will be used.
     * @a prefix is the part between the file name and the locale part. In the example above it is
     * @c "_", if it is not set the default @c "." will be used. The
     * @a suffix is the file name suffix that defaults to <code>".qm"</code>.
     *
     * @sa addTranslator(), loadTranslationsFromDir(), loadTranslationsFromDirs()
     * @sa @ref translations
     *
     * @since %Cuteylst 2.0.0
     */
    void loadTranslations(const QString &filename,
                          const QString &directory = {},
                          const QString &prefix    = {},
                          const QString &suffix    = {});

    /**
     * Loads translations for a specific @a filename from a single directory and returns a list of
     * added locales.
     *
     * This can be used to load translations for a specific component or application if the
     * translation file names follow a common schema.
     *
     * Let us assume you organised your translation files as follows:
     * @li @c /usr/share/myapp/translations/myapp_de.qm
     * @li @c /usr/share/myapp/translations/myapp_pt_BR.qm
     * @li @c ...
     *
     * You can then use loadTranslationsFromDir() in your reimplementation of Application::init() as
     * follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *      loadTranslationsFromDir(QStringLiteral("myapp"),
     *                              QStringLiteral("/usr/share/myapp/translations"),
     *                              QStringLiteral("_"));
     * }
     * @endcode
     *
     * If @a directory is empty, the default directory, set by <code>-DI18NDIR</code>, will be used.
     * @a prefix is the part between the file name and the locale part. In the example above it is
     * @c "_", if it is not set the default @c "." will be used. The
     * @a suffix is the file name suffix that defaults to <code>".qm"</code>.
     *
     * @sa addTranslator(), loadTranslationsFromDirs()
     * @sa @ref translations
     *
     * @since %Cuteylst 2.1.0
     */
    QVector<QLocale> loadTranslationsFromDir(const QString &filename,
                                             const QString &directory = {},
                                             const QString &prefix    = QStringLiteral("."),
                                             const QString &suffix    = QStringLiteral(".qm"));

    /**
     * Loads translations for a specific @a filename from a directory structure under @a directory
     * and returns a list of added locales.
     *
     * This can be used to load translations for a specific component or application if the the
     * translation files are organized in subdirectories named after locale codes. Let us assume you
     * organised your translation files as follows:
     * @li @c /usr/share/locale/de/LC_MESSAGES/myapp.qm
     * @li @c /usr/share/locale/pt_BR/LC_MESSAGES/myapp.qm
     * @li @c ...
     *
     * You can then use loadTranslationsFromDirs() in your reimplementation of Application::init()
     * as follows:
     * @code{.cpp}
     * bool MyApp::init()
     * {
     *     loadTranslationsFromDirs(QStringLiteral("/usr/share/locale"),
     *                              QStringLiteral("LC_MESSAGES/myapp.qm"));
     * }
     * @endcode
     *
     * @sa addTranslator(), loadTranslationsFromDir()
     * @sa @ref translations
     *
     * @since %Cutelyst 2.1.0
     */
    QVector<QLocale> loadTranslationsFromDirs(const QString &directory, const QString &filename);

    /**
     * Returns the default locale that will be set to the \link Context::locale() locale()\endlink
     * of newly created Context objects. By default this will be QLocale(English, Latin, United
     * States).
     * @sa setDefaultLocale()
     * @sa @ref translations
     * @since %Cutelyst 4.0.0
     */
    [[nodiscard]] QLocale defaultLocale() const noexcept;

    /**
     * Sets the default locale that will be set to the \link Context::locale() locale()\endlink
     * of newly created Context objects. By default this will be QLocale(English, Latin, United
     * States).
     * @sa defaultLocale()
     * @sa @ref translations
     * @since %Cutelyst 4.0.0
     */
    void setDefaultLocale(const QLocale &locale);

protected:
    /**
     * Do your application initialization here, if your
     * application should not proceed log some information
     * that might help on debugging and return \c false.
     *
     * For example if your application only works with
     * PostgeSQL and the Qt driver is not available it
     * makes sense to fail here. However you should not
     * initialize resouces that cannot be shared among
     * process.
     *
     * \sa postFork
     *
     * @return \c true if your application was successfuly initted
     */
    virtual bool init();

    /**
     * This method is called after the engine forks
     *
     * After the web engine forks itself it will call
     * this function so that you can initialize resources
     * that can’t be shared with the parent process, namely
     * sockets and file descriptors.
     *
     * A good example of usage of this function is when
     * opening a connection to the database which can’t
     * be shared with other processes and should probably
     * make this function return \c false if it fails to open.
     *
     * Default implementation returns \c true.
     *
     * @return \c false if the engine should not use this process
     */
    virtual bool postFork();

    /**
     * This is the HTTP default response headers that each request gets.
     *
     * Do not change it after the application has started.
     */
    Headers &defaultHeaders() noexcept;

    /**
     * Adds a X-Cutelyst header with our version on each request.
     */
    void addXCutelystVersionHeader();

    /**
     * Registers a global plugin ie. one that doesn’t need
     * to be created explicity for a single request and returns
     * true on plugin->isApplicationPlugin();
     *
     * @return \c true if the plugin could be registered
     */
    bool registerPlugin(Plugin *plugin);

    /**
     * This method registers a Controller class which
     * is responsible for handlying requests,
     * since they are reused between multiple requests.
     * Be aware of not storing data there, instead you
     * might want to use a session plugin or the stash.
     *
     * @param controller the Controller class
     * @return \c true if succeeded
     */
    bool registerController(Controller *controller);

    /**
     * This method registers a \ref plugins-view class which
     * is responsible for rendering requests.
     *
     * @param view the \ref plugins-view class
     * @return \c true if succeeded
     */
    bool registerView(View *view);

    /**
     * Registers a custom DispatchType. If none is registered,
     * all the built-in dispatcher types will be registered.
     */
    bool registerDispatcher(DispatchType *dispatcher);

Q_SIGNALS:
    /**
     * This signal is emitted before the Dispatcher
     * is called to find an action.
     * It’s useful if you need to intercept requests
     * before they are dispatched.
     * Always check \a skipMethod and return if it’s \c true.
     * In case you want to stop further processing set
     * \a skipMethod to \c true.
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
     * and before application forks and postFork() is called.
     */
    void preForked(Cutelyst::Application *app);

    /**
     * This signal is emitted after postFork() is called.
     */
    void postForked(Cutelyst::Application *app);

    /**
     * This signal is likely to be emitted when the worker process should
     * stop. At this point the application has a limited time to finish it’s
     * operations. If a timeout is reached the application will get killed.
     */
    void shuttingDown(Cutelyst::Application *app);

protected:
    /**
     * Change the \a value of the configuration \a key.
     *
     * This will change or add keys to the configuration read from the \c %Cutelyst section
     * of your \ref configuration "configuration file". The set values are not written to
     * the file.
     *
     * \warning You should never call this from random parts of the code as a way to store
     * shareable data, it should only be called by a subclass
     */
    void setConfig(const QString &key, const QVariant &value);

    friend class Engine;
    friend class Context;

    /**
     * Called by the Engine to setup the internal data.
     */
    bool setup(Engine *engine);

    /**
     * Called by the Engine to handle a new Request object.
     */
    void handleRequest(Cutelyst::EngineRequest *request);

    /**
     * Called by the Engine once post fork happened.
     */
    bool enginePostFork();

    ApplicationPrivate *d_ptr;
};

} // namespace Cutelyst

#define CutelystApplicationInterface_iid "org.cutelyst.CutelystApplicationInterface"

Q_DECLARE_INTERFACE(Cutelyst::Application, CutelystApplicationInterface_iid)

#endif // CUTELYST_APPLICATION_H
