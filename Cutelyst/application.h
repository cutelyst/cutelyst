/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#include <QtCore/qobject.h>
#include <QtCore/qvariant.h>

#include <Cutelyst/cutelyst_global.h>

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

/**
 * @brief Cutelyst::Application - The Cutelyst Application
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
     * use config(), do that in init()
     */
    explicit Application(QObject *parent = nullptr);
    virtual ~Application();

    QList<Controller *> controllers() const;

    View *view(const QString &name = QString()) const;

    QVariant config(const QString &key, const QVariant &defaultValue = QVariant()) const;

    Dispatcher *dispatcher() const;

    QList<DispatchType *> dispatchers() const;

    /**
     * User configuration for the application
     * @return A variant hash with configuration settings
     */
    QVariantMap config() const;

    /**
     * Merges path with config("HOME") and returns a QDir object.
     */
    QString pathTo(const QStringList &path) const;

    bool inited() const;

    Engine *engine() const;

    Component *createComponentPlugin(const QString &name, QObject *parent = nullptr);

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
     * @return True if your application was successfuly initted
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
     * This is the HTTP default headers appended to each request
     *
     * Do not change it after the application has started.
     */
    Headers &defaultHeaders();

    /**
     * Registers a global plugin ie one that doesn't need
     * to be created explicity for a single request and returns
     * true on plugin->isApplicationPlugin();
     *
     * @return True if the plugin could be registered
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
     * @return True if succeeded
     */
    bool registerController(Controller *controller);

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

    void preForked(Application *app);

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
    bool setup(Engine *engine);
    void handleRequest(Request *req);
    bool enginePostFork();

    ApplicationPrivate *d_ptr;
};

}

#define CutelystApplicationInterface_iid "org.cutelyst.CutelystApplicationInterface"

Q_DECLARE_INTERFACE(Cutelyst::Application, CutelystApplicationInterface_iid)

#endif // CUTELYST_APPLICATION_H
