/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_CONTEXT_H
#define CUTELYST_CONTEXT_H

#include <Cutelyst/async.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/request.h>

#include <QtCore/QObject>
#include <QtCore/QStack>
#include <QtCore/QStringList>
#include <QtCore/QUrl>
#include <QtCore/QVariant>

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
    Q_PROPERTY(Action *action READ action CONSTANT)
    Q_PROPERTY(QString actionName READ actionName CONSTANT)
    Q_PROPERTY(QString ns READ ns CONSTANT)
    Q_PROPERTY(QString namespace READ ns CONSTANT)
    Q_PROPERTY(Request *req READ request CONSTANT)
    Q_PROPERTY(Request *request READ request CONSTANT)
    Q_PROPERTY(Controller *controller READ controller CONSTANT)
    Q_PROPERTY(QString controllerName READ controllerName CONSTANT)
    Q_PROPERTY(QVariantMap config READ config CONSTANT)
    Q_PROPERTY(bool state READ state CONSTANT)
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
    bool error() const noexcept;

    /*!
     * Sets an error string and try to stop
     */
    void error(const QString &error);

    /*!
     * Returns a list of errors that were defined
     */
    QStringList errors() const noexcept;

    /**
     * Contains the return value of the last executed action.
     */
    bool state() const noexcept;

    /*!
     * Sets the state of the current executed action, setting to false
     * will make the dispatcher skip non processed actions.
     */
    void setState(bool state) noexcept;

    /**
     * Returns the engine instance. See Cutelyst::Engine
     */
    Engine *engine() const noexcept;

    /**
     * Returns the application instance. See Cutelyst::Application
     */
    Application *app() const noexcept;

    /**
     * Returns the current Cutelyst::Response object, see there for details.
     */
    Response *response() const noexcept;

    /**
     * Returns the current Cutelyst::Response object, see there for details.
     */
    Response *res() const noexcept;

    /**
     * Returns a pointer to the current action
     */
    Action *action() const noexcept;

    /**
     * Returns the private name of the current action
     */
    QString actionName() const noexcept;

    /**
     * Returns the namespace of the current action.
     * i.e. the URI prefix corresponding to the controller
     * of the current action. For example:
     * // a class named FooBar which inherits Controller
     * c->ns(); // returns 'foo/bar'
     */
    QString ns() const noexcept;

    /**
     * Returns the current Request object containing
     * information about the client request Request
     */
    Request *request() const noexcept;

    /**
     * Short for request()
     */
    Request *req() const noexcept;

    /**
     * Returns the dispatcher instance. See Cutelyst::Dispatcher
     */
    Dispatcher *dispatcher() const noexcept;

    /**
     * The current controller name
     */
    QString controllerName() const;

    /**
     * Returns the current controller
     */
    Controller *controller() const noexcept;

    /**
     * Returns the controller by name, or nullptr
     * if the controller is not found
     */
    Controller *controller(const QString &name) const;

    /**
     * Returns the view with name name or nullptr if not found
     */
    View *view(const QString &name) const;

    /**
     * Returns the view with name name or nullptr if not found
     */
    View *view(QStringView name = {}) const;

    /**
     * Returns the view set to be used
     * for rendering this request, if one
     * is set by setView() or nullptr if none was set
     */
    View *customView() const noexcept;

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
    bool setCustomView(const QString &name);

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
     * If a given key is present it will be replaced
     *
     * \code{.cpp}
     * c->stash({
     *              {"foo", 10},
     *              {"bar", QStringLiteral("my stash value")}
     *            });
     * \endcode
     */
    void stash(const QVariantHash &unite);

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
     * A convenient method to retrieve a single value with a default value from the stash
     */
    QVariant stash(const QString &key, const QVariant &defaultValue) const;

    /**
     * Removes the item with the key from the stash and returns the value associated with it.
     * If the item does not exist in the stash, the function simply returns a default-constructed value.
     * If you don't use the return value, stashRemove() is more efficient.
     */
    QVariant stashTake(const QString &key);

    /**
     * Removes the item that has the key from the stash.
     * Returns true if any item was removed removed.
     */
    bool stashRemove(const QString &key);

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
    QStack<Component *> stack() const noexcept;

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
    QUrl uriFor(const QString &path               = QString(),
                const QStringList &args           = QStringList(),
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
                const QStringList &captures       = QStringList(),
                const QStringList &args           = QStringList(),
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
                      const QStringList &captures       = QStringList(),
                      const QStringList &args           = QStringList(),
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
    bool detached() const noexcept;

    /**
     * The same as forward(action)
     *
     * When called with no arguments it escapes the processing chain entirely.
     */
    void detach(Action *action = nullptr);

    /**
     * Detaches the processing chain telling the Engine that
     * the request is not finished yet.
     *
     * The scoped \sa Async class can make handlying async requests easier.
     *
     * It's often useful to call async API's, while convenient the use of QEventLoop
     * will only work for the first request or lead to a crash due stacking of calls.
     *
     * This method, tells the Engine that this request is not finished yet, making
     * it return to the event loop to process other requests or the task that was
     * created prior to calling this.
     *
     * Once done call attachAsync() in order to process the remaining of the action chain.
     */
    void detachAsync() noexcept;

    /*!
     * \brief attachAsync
     *
     * The scoped \sa Async class can make handlying async requests easier.
     *
     * Reattaches to the remaining actions
     */
    void attachAsync();

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
    Action *getAction(const QString &action, const QString &ns = {}) const;

    /**
     * Gets all actions of a given name in a namespace and all parent namespaces.
     */
    QVector<Action *> getActions(const QString &action, const QString &ns = {}) const;

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
    QLocale locale() const noexcept;

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
    QVariant config(const QString &key, const QVariant &defaultValue = {}) const;

    /**
     * Returns a configuration mapping for all configuration read
     */
    QVariantMap config() const noexcept;

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

public Q_SLOTS:
    /*!
     * \brief finalize the request right away this is automatically called
     * at the end of the actions chain
     */
    void finalize();

protected:
    /*!
     * Constructs a new Context object using private implementation.
     */
    Context(ContextPrivate *priv);

    friend class Application;
    friend class Action;
    friend class ActionChain;
    friend class DispatchType;
    friend class Plugin;
    friend class Engine;
    friend class Controller;
    friend class Async;
    ContextPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Context)
};

inline QUrl Context::uriFor(const QString &path, const ParamsMultiMap &queryValues) const
{
    return uriFor(path, QStringList(), queryValues);
}

inline QUrl Context::uriFor(Action *action, const ParamsMultiMap &queryValues) const
{
    return uriFor(action, QStringList(), QStringList(), queryValues);
}

inline QUrl Context::uriForAction(const QString &path, const ParamsMultiMap &queryValues) const
{
    return uriForAction(path, QStringList(), QStringList(), queryValues);
}

} // namespace Cutelyst

Q_DECLARE_METATYPE(Cutelyst::Context *)

#endif // CUTELYST_CONTEXT_H
