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

#ifndef CUTELYST_CONTEXT_H
#define CUTELYST_CONTEXT_H

#include <QtCore/QObject>
#include <QtCore/QVariant>
#include <QtCore/QUrl>
#include <QtCore/QStringList>
#include <QtCore/QStack>

#include <Cutelyst/request.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class Action;
class Application;
class Component;
class Engine;
class Response;
class Dispatcher;
class Controller;
class View;
class Stats;
class Plugin;
class ContextPrivate;

/*! \class Context context.h Cutelyst/Context
 * @brief The %Cutelyst %Context
 *
 * This is the context class that glues Request and Response plus
 * some helper methods.
 */
class CUTELYST_LIBRARY Context : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Action* action READ action)
    Q_PROPERTY(QString actionName READ actionName)
    Q_PROPERTY(QString ns READ ns)
    Q_PROPERTY(QString namespace READ ns)
    Q_PROPERTY(Request *req READ request)
    Q_PROPERTY(Request *request READ request)
    Q_PROPERTY(Controller *controller READ controller)
    Q_PROPERTY(QString controllerName READ controllerName)
    Q_PROPERTY(QVariantMap config READ config)
    Q_PROPERTY(bool state READ state)
public:
    /*!
     * Constructs a new DUMMY Context object that is child of Application
     * This currently is experimental to allow non network events (such as database notification)
     * to be able to use our infrastructure
     */
    Context(Application *app);
    virtual ~Context();

    /*!
     * Returns true if an error was set.
     */
    bool error() const;

    /*!
     * Sets an error string and try to stop
     */
    void error(const QString &error);

    /*!
     * Returns a list of errors that were defined
     */
    QStringList errors() const;

    /**
     * Contains the return value of the last executed action.
     */
    bool state() const;

    /*!
     * Sets the state of the current executed action, setting to false
     * will make the dispatcher skip non processed actions.
     */
    void setState(bool state);

    /**
     * Returns the engine instance. See Cutelyst::Engine
     */
    Engine *engine() const;

    /**
     * Returns the application instance. See Cutelyst::Application
     */
    Application *app() const;

    /**
     * Returns the current Cutelyst::Response object, see there for details.
     */
    Response *response() const;

    /**
     * Returns the current Cutelyst::Response object, see there for details.
     */
    Response *res() const;

    /**
     * Returns a pointer to the current action
     */
    Action *action() const;

    /**
     * Returns the private name of the current action
     */
    QString actionName() const;

    /**
     * Returns the namespace of the current action.
     * i.e. the URI prefix corresponding to the controller
     * of the current action. For example:
     * // a class named FooBar which inherits Controller
     * c->ns(); // returns 'foo/bar'
     */
    QString ns() const;

    /**
     * Returns the current Request object containing
     * information about the client request Request
     */
    Request *request() const;

    /**
     * Short for request()
     */
    Request *req() const;

    /**
     * Returns the dispatcher instance. See Cutelyst::Dispatcher
     */
    Dispatcher *dispatcher() const;

    /**
     * The current controller name
     */
    QString controllerName() const;

    /**
     * Returns the current controller
     */
    Controller *controller() const;

    /**
     * Returns the controller by name, or 0
     * if the controller is not found
     */
    Controller *controller(const QString &name) const;

    /**
     * Returns the view set to be used
     * for rendering this request, if one
     * is set by setView() or 0 if none was set
     */
    View *view() const;

    /**
     * Returns the view with name name
     */
    // TODO find a new name for the above method
    View *view(const QString &name) const;

    /**
     * Defines the view to be used to render
     * the request, it must be previously
     * be registered by Cutelyst::Application.
     *
     * Action classes like RenderView will use
     * this value to overwrite their settings.
     *
     * Returns true if a view with the given
     * name was found
     */
    bool setView(const QString &name);

    /**
     * You can set hash keys by passing arguments,
     * that will be united with the stash,
     * which may be used to store data and pass it between
     * components during a request.
     *
     * The stash is automatically sent to the view.
     * The stash is cleared at the end of a request;
     * it cannot be used for persistent storage
     * (for this you must use a session; see Cutelyst::Plugin::Session
     * for a complete system integrated with Cutelyst).
     *
     * \code{.cpp}
     * c->stash({
     *              {"foo", 10},
     *              {"bar", QStringLiteral("my stash value")}
     *            });
     * \endcode
     */
    inline void stash(const QVariantHash &unite);

    /**
     * Returns a QVariantHash reference to the stash,
     * which may be used to store data and pass it between
     * components during a request.
     *
     * The stash is automatically sent to the view.
     * The stash is cleared at the end of a request;
     * it cannot be used for persistent storage
     * (for this you must use a session; see Cutelyst::Plugin::Session
     * for a complete system integrated with Cutelyst).
     */
    QVariantHash &stash();

    /**
     * A convenient method to retrieve a single value from the stash
     */
    QVariant stash(const QString &key) const;

    /**
     * A convenient method to set a single value to the stash
     */
    void setStash(const QString &key, const QVariant &value);

    /**
     * A convenient method to set a single ParamsMultiMap to the stash
     */
    void setStash(const QString &key, const ParamsMultiMap &map);

    /**
     * Returns the internal execution stack (actions that are currently executing).
     */
    QStack<Component *> stack() const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     *
     * The first argument is taken as a public URI path relative
     * c->ns (if it doesn't begin with a forward slash) or
     * relative to the application root (if it does). It is then merged with
     * c->request()->base() any \p args are appended as additional path
     * components; and any queryValues> are appended as "?foo=bar" parameters.
     */
    QUrl uriFor(const QString &path = QString(),
                const QStringList &args = QStringList(),
                const ParamsMultiMap &queryValues = ParamsMultiMap()) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     *
     * The first argument is taken as a public URI path relative
     * c->ns (if it doesn't begin with a forward slash) or
     * relative to the application root (if it does). It is then merged with
     * c->request()->base() and any queryValues> are appended as "?foo=bar" parameters.
     */
    inline QUrl uriFor(const QString &path,
                       const ParamsMultiMap &queryValues) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     *
     * If no arguments are provided, the URI for the current action is returned.
     * To return the current action and also provide \p args, use
     * c->uriFor(c->action(), args).
     */
    QUrl uriFor(Action *action,
                const QStringList &captures = QStringList(),
                const QStringList &args = QStringList(),
                const ParamsMultiMap &queryValues = ParamsMultiMap()) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     */
    inline QUrl uriFor(Action *action,
                       const ParamsMultiMap &queryValues) const;

    /**
     * A private path to the Cutelyst action you want to create a URI for.
     *
     * This is a shortcut for calling c->dispatcher()->getActionByPath(path)
     * and passing the resulting action and the remaining arguments to c->uri_for.
     *
     * Note that although the path looks like a URI that dispatches to the wanted action,
     * it is not a URI, but an internal path to that action.
     *
     * For example, if the action looks like:
     * \code{.h}
     * class Users : public Cutelyst::Controller
     * {
     * public:
     *   C_ATTR(lst, :Path(the-list))
     *   void lst(Context *c);
     * };
     * \endcode
     *
     * You can use:
     * c->uriForAction('/users/lst');
     * and it will create the URI /users/the-list.
     */
    QUrl uriForAction(const QString &path,
                      const QStringList &captures = QStringList(),
                      const QStringList &args = QStringList(),
                      const ParamsMultiMap &queryValues = ParamsMultiMap()) const;

    /**
     * A convenience method for the uriForAction() without the arguments parameter
     */
    inline QUrl uriForAction(const QString &path,
                             const ParamsMultiMap &queryValues) const;

    /**
     * Returns true if the last executed Action requested
     * that the processing be escaped.
     */
    bool detached() const;

    /**
     * The same as forward(action), but doesn't return to the previous
     * action when processing is finished.
     * When called with no arguments it escapes the processing chain entirely.
     */
    void detach(Action *action = nullptr);

    /**
     * This is one way of calling another action (method) in the same or
     * a different controller. You can also use directly call another method
     * to the same or a different controller.
     *
     * The main difference is that 'forward' uses some of the Cutelyst request
     * cycle overhead, including debugging, which may be useful to you. On the
     * other hand, there are some complications to using 'forward', restrictions
     * on values returned from 'forward', and it may not handle errors as you prefer.
     * Whether you use 'forward' or not is up to you; it is not considered superior to
     * the other ways to call a method.
     *
     * forward calls Component::execute.
     *
     * Keep in mind that the End() method used is that of the caller action.
     * So a c->detach() inside a forwarded action would run the End() method from
     * the original action requested.
     */
    bool forward(Component *component);

    /**
     * This is one way of calling another action (method) in the same or
     * a different controller. You can also use directly call another method
     * to the same or a different controller.
     *
     * The main difference is that 'forward' uses some of the Cutelyst request
     * cycle overhead, including debugging, which may be useful to you. On the
     * other hand, there are some complications to using 'forward', restrictions
     * on values returned from 'forward', and it may not handle errors as you prefer.
     * Whether you use 'forward' or not is up to you; it is not considered superior to
     * the other ways to call a method.
     *
     * forward calls another action, by its private name.
     *
     * Keep in mind that the End() method used is that of the caller action.
     * So a c->detach() inside a forwarded action would run the End() method from
     * the original action requested.
     */
    bool forward(const QString &action);

    /**
     * Gets an action in a given namespace.
     */
    // TODO C2 mark as const
    Action *getAction(const QString &action, const QString &ns = QString());

    /**
     * Gets all actions of a given name in a namespace and all parent namespaces.
     */
    // TODO C2 mark as const
    QVector<Action *> getActions(const QString &action, const QString &ns = QString());

    /**
     * Returns all registered plugins
     */
    // TODO C2 mark as const
    QVector<Plugin *> plugins();

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
     * Execute an action. Errors are available via error().
     */
    bool execute(Component *code);

    /**
     * Returns the current locale to be used when processing Views
     * or translating user messages.
     *
     * If not explicity set by setLocale it will use the QLocale::setDefault(),
     * or QLocale::system() if not set.
     */
    QLocale locale() const;

    /**
     * Defines the current locale to be used when processing Views
     * or translating user messages.
     *
     * Setting a locale on a web application can be done in many ways,
     * so it's up to the developer to decide which one to use.
     *
     * For example it's possible to try to guess the user locale with
     * the request header Accept-Language, and  or use the chained dispatcher to first
     * match the locale as in "example.com/pt-br/some_action", and or store
     * the locale into a cookie or session.
     *
     * Be sure to set it as soon as possible so that all content can be properly localized.
     */
    void setLocale(const QLocale &locale);

    /**
     * Returns a configuration value for key with an optional default value
     */
    QVariant config(const QString &key, const QVariant &defaultValue = QVariant()) const;

    /**
     * Returns a configuration mapping for all configuration read
     */
    QVariantMap config() const;

    /**
     * Pointer to internal engine data about the current request.
     * \note It's only used by Engines subclasses, Application code should not use it.
     */
    void *engineData();

    /**
     * Translates the \a sourceText for the given \a context into the language defined by locale().
     *
     * See Application::addTranslator() for information about installation of translators. Internally
     * this function will use QTranslator::translate().
     *
     * \code{.cpp}
     * void MyController::index(Context *c)
     * {
     *      c->res()->setBody(c->translate("MyController", "You are on the index page."));
     * }
     * \endcode
     */
    QString translate(const char *context, const char *sourceText, const char *disambiguation = nullptr, int n = -1) const;

protected:
    /*!
     * Constructs a new Context object using private implementation.
     */
    Context(ContextPrivate *priv);

    friend class Application;
    friend class Action;
    friend class DispatchType;
    friend class Plugin;
    friend class Engine;
    ContextPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Context)
};

inline void Context::stash(const QVariantHash &unite)
{ stash().unite(unite); }

inline QUrl Context::uriFor(const QString &path, const ParamsMultiMap &queryValues) const
{ return uriFor(path, QStringList(), queryValues); }

inline QUrl Context::uriFor(Action *action, const ParamsMultiMap &queryValues) const
{ return uriFor(action, QStringList(), QStringList(), queryValues); }

inline QUrl Context::uriForAction(const QString &path, const ParamsMultiMap &queryValues) const
{ return uriForAction(path, QStringList(), QStringList(), queryValues); }

}

Q_DECLARE_METATYPE(Cutelyst::Context *)

#endif // CUTELYST_CONTEXT_H
