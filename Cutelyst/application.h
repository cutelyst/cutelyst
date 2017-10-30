/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_APPLICATION_H
#define CUTELYST_APPLICATION_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QLocale>
#include <QtCore/QVector>

#include <Cutelyst/cutelyst_global.h>

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
    QVector<Controller *> controllers() const;

    /**
     * Returns the view specified by \p name, if no view is found nullptr is returned.
     */
    View *view(const QString &name = QString()) const;

    /**
     * Returns application config specified by \p key, with a possible default value.
     */
    QVariant config(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /**
     * Returns the dispatcher class.
     */
    Dispatcher *dispatcher() const;

    /**
     * Returns a list with all registered dispachers.
     *
     * The list might only be complete after application has been setup.
     */
    QVector<DispatchType *> dispatchers() const;

    /**
     * Returns all registered plugins
     */
    QVector<Plugin *> plugins() const;

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
    QVariantMap config() const;

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
    bool inited() const;

    /**
     * Returns current engine that is generating requests.
     */
    Engine *engine() const;

    /**
     * Tries to load a plugin in Cutelyst default plugin directory with \p parent as it's parent.
     * A nullptr is returned in case of failure.
     */
    Component *createComponentPlugin(const QString &name, QObject *parent = nullptr);

    /**
     * Returns cutelyst version.
     */
    static const char *cutelystVersion();

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
    Headers &defaultHeaders();

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
    void beforePrepareAction(Context *c, bool *skipMethod);

    /**
     * This signal is emitted right after the Dispatcher
     * returns the Action that will be executed.
     */
    void beforeDispatch(Context *c);

    /**
     * This signal is emitted right after the Action
     * found by the dispatcher got executed.
     */
    void afterDispatch(Context *c);

    /**
     * This signal is emitted right after application has been setup
     * and before application forks and \sa postFork() is called.
     */
    void preForked(Application *app);

    /**
     * This signal is emitted after before \sa postFork() is called.
     */
    void postForked(Application *app);

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
    Q_DECL_DEPRECATED
    void handleRequest(Request *req);

    /*!
     * Called by the Engine to handle a new Request object
     */
    Context *handleRequest2(Request *req);

    /*!
     * Called by the Engine once post fork happened
     */
    bool enginePostFork();

    ApplicationPrivate *d_ptr;
};

}

#define CutelystApplicationInterface_iid "org.cutelyst.CutelystApplicationInterface"

Q_DECLARE_INTERFACE(Cutelyst::Application, CutelystApplicationInterface_iid)

#endif // CUTELYST_APPLICATION_H
