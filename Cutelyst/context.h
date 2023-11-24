/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

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

/**
 * \ingroup core
 * \class Context context.h Cutelyst/Context
 * \brief The %Cutelyst %Context.
 *
 * This is the context class that glues Request and Response plus
 * some helper methods.
 *
 * \logcat{core}
 */
class CUTELYST_LIBRARY Context : public QObject
{
    Q_OBJECT
    /**
     * Pointer to the current action.
     */
    Q_PROPERTY(Action *action READ action CONSTANT)
    /**
     * Private name of the current action
     */
    Q_PROPERTY(QString actionName READ actionName CONSTANT)
    /**
     * The namespace of the current action.
     * \sa ns()
     */
    Q_PROPERTY(QString ns READ ns CONSTANT)
    /**
     * The namespace of the current action.
     * \sa ns()
     */
    Q_PROPERTY(QString namespace READ ns CONSTANT)
    /**
     * The current Request object containing
     * information about the client request.
     */
    Q_PROPERTY(Request *req READ request CONSTANT)
    /**
     * The current Request object containing
     * information about the client request.
     */
    Q_PROPERTY(Request *request READ request CONSTANT)
    /**
     * The current controller.
     */
    Q_PROPERTY(Controller *controller READ controller CONSTANT)
    /**
     * The current controller name.
     */
    Q_PROPERTY(QString controllerName READ controllerName CONSTANT)
    /**
     * Mapping for all configuration entries read from the \c %Cutelyst
     * configuration section.
     *
     * \sa \ref configuration
     */
    Q_PROPERTY(QVariantMap config READ config CONSTANT)
    /**
     * Holds the return value of the last executed action.
     */
    Q_PROPERTY(bool state READ state CONSTANT)
public:
    /**
     * Constructs a new DUMMY Context object that is child of Application.
     *
     * \warning This currently is experimental to allow non network events (such as database
     * notification) to be able to use our infrastructure.
     */
    Context(Application *app);

    /**
     * Destroys the %Context object.
     * This will also delete the associated Request and Response.
     */
    virtual ~Context();

    /**
     * Returns \c true if an error was set.
     * \sa appendError() errors()
     */
    [[nodiscard]] bool error() const noexcept;

    /**
     * Sets an \a error string and tries to stop.
     * \sa error() errors()
     */
    void appendError(const QString &error);

    /**
     * Returns a list of errors that were defined.
     * \sa error() appendError()
     */
    [[nodiscard]] QStringList errors() const noexcept;

    /**
     * Contains the return value of the last executed action.
     */
    [[nodiscard]] bool state() const noexcept;

    /**
     * Sets the \a state of the current executed action. Setting to \c false
     * will make the dispatcher skip non processed actions.
     */
    void setState(bool state) noexcept;

    /**
     * Returns the engine instance. See Cutelyst::Engine.
     */
    [[nodiscard]] Engine *engine() const noexcept;

    /**
     * Returns the application instance. See Cutelyst::Application.
     */
    [[nodiscard]] Application *app() const noexcept;

    /**
     * Returns the current Cutelyst::Response object, see there for details.
     */
    [[nodiscard]] Response *response() const noexcept;

    /**
     * Returns the current Cutelyst::Response object, see there for details.
     */
    [[nodiscard]] Response *res() const noexcept;

    /**
     * Returns a pointer to the current action.
     */
    [[nodiscard]] Action *action() const noexcept;

    /**
     * Returns the private name of the current action.
     */
    [[nodiscard]] QString actionName() const noexcept;

    /**
     * Returns the namespace of the current action.
     * I.e. the URI prefix corresponding to the controller
     * of the current action.
     *
     * For example: on a class named \c FooBar which inherits Controller
     * this will return \c 'foo/bar'.
     */
    [[nodiscard]] QString ns() const noexcept;

    /**
     * Returns the current Request object containing
     * information about the client request.
     */
    [[nodiscard]] Request *request() const noexcept;

    /**
     * Short for request().
     */
    [[nodiscard]] Request *req() const noexcept;

    /**
     * Returns the dispatcher instance. See Cutelyst::Dispatcher.
     */
    [[nodiscard]] Dispatcher *dispatcher() const noexcept;

    /**
     * Returns the current controller name.
     */
    [[nodiscard]] QString controllerName() const noexcept;

    /**
     * Returns the current controller.
     */
    [[nodiscard]] Controller *controller() const noexcept;

    /**
     * Returns the controller by \a name or \c nullptr
     * if the controller is not found.
     */
    [[nodiscard]] Controller *controller(QStringView name) const;

    /**
     * Returns the \ref plugins-view with \a name or \c nullptr if not found.
     */
    [[nodiscard]] View *view(QStringView name = {}) const;

    /**
     * Returns the \ref plugins-view set to be used
     * for rendering this request, if one
     * is set by setCustomView() or \c nullptr if none was set.
     */
    [[nodiscard]] View *customView() const noexcept;

    /**
     * Defines the \ref plugins-view to be used to render
     * the request, it must be previously
     * be registered by Cutelyst::Application.
     *
     * Action classes like RenderView will use
     * this value to overwrite their settings.
     *
     * Returns \c true if a view with the given
     * \a name was found.
     */
    bool setCustomView(QStringView name);

    /**
     * You can set hash keys by passing arguments,
     * that will be united with the stash,
     * which may be used to store data and pass it between
     * components during a request.
     *
     * The stash is automatically sent to the \ref plugins-view.
     * The stash is cleared at the end of a request;
     * it cannot be used for persistent storage
     * (for this you must use a session; see \ref plugins-session
     * for a complete system integrated with %Cutelyst).
     *
     * If a given key is present it will be replaced.
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
     * (for this you must use a session; see \ref plugins-session
     * for a complete system integrated with %Cutelyst).
     */
    [[nodiscard]] QVariantHash &stash();

    /**
     * A convenient method to retrieve a single value from the stash by \a key.
     */
    [[nodiscard]] QVariant stash(const QString &key) const;

    /**
     * A convenient method to retrieve a single value with the stash by \a key. If
     * \a key is not found, \a defaultValue will be returned.
     */
    [[nodiscard]] QVariant stash(const QString &key, const QVariant &defaultValue) const;

    /**
     * Removes the item with the \a key from the stash and returns the value associated with it.
     * If the item does not exist in the stash, the function simply returns a default-constructed
     * value. If you don’t use the return value, stashRemove() is more efficient.
     */
    QVariant stashTake(const QString &key);

    /**
     * Removes the item that has the \a key from the stash.
     * Returns \c true if any item was removed.
     */
    bool stashRemove(const QString &key);

    /**
     * A convenient method to set a single \a value to the stash on \a key.
     */
    void setStash(const QString &key, const QVariant &value);

    /**
     * A convenient method to set a single ParamsMultiMap \a map to the stash on \a key.
     */
    void setStash(const QString &key, const ParamsMultiMap &map);

    /**
     * Returns the internal execution stack (actions that are currently executing).
     */
    [[nodiscard]] QStack<Component *> stack() const noexcept;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided \a path, and the additional arguments \a args and query parameters
     * \a queryValues provided. When used as a string, provides a textual URI.
     *
     * The first argument is taken as a public URI path relative
     * \link ns() c->ns()\endlink (if it doesn’t begin with a forward slash) or
     * relative to the application root (if it does). It is then merged with
     * \link Request::base() c->request()->base()\endlink any \a args are appended
     * as additional path components; and any \a queryValues are appended as \c "?foo=bar"
     * parameters.
     */
    [[nodiscard]] QUrl uriFor(const QString &path               = {},
                              const QStringList &args           = {},
                              const ParamsMultiMap &queryValues = {}) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided \a path and the query parameters \a queryValues provided.
     * When used as a string, provides a textual URI.
     *
     * The first argument is taken as a public URI path relative
     * \link ns() c->ns()\endlink (if it doesn’t begin with a forward slash) or
     * relative to the application root (if it does). It is then merged with
     * \link Request::base() c->request()->base()\endlink and any \a queryValues
     * are appended as \c "?foo=bar" parameters.
     */
    [[nodiscard]] inline QUrl uriFor(const QString &path, const ParamsMultiMap &queryValues) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided \a action, \a captures, arguments \a args and query parameters
     * \a queryValues. When used as a string, provides a textual URI.
     *
     * If \a action is a \c nullptr, the URI for the current action is returned.
     * To return the current action and also provide \p args, use
     * c->uriFor(c->action(), args).
     */
    [[nodiscard]] QUrl uriFor(Action *action,
                              const QStringList &captures       = {},
                              const QStringList &args           = {},
                              const ParamsMultiMap &queryValues = {}) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided \a action and the query parameters \a queryValues provided.
     * When used as a string, provides a textual URI.
     */
    [[nodiscard]] inline QUrl uriFor(Action *action, const ParamsMultiMap &queryValues) const;

    /**
     * A private path to the Cutelyst action you want to create a URI for.
     *
     * This is a shortcut for calling c->dispatcher()->getActionByPath(path)
     * and passing the resulting action and the remaining arguments to c->uriFor().
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
    [[nodiscard]] QUrl uriForAction(QStringView path,
                                    const QStringList &captures       = {},
                                    const QStringList &args           = {},
                                    const ParamsMultiMap &queryValues = {}) const;

    /**
     * A convenience method for the uriForAction() without the arguments parameter.
     */
    [[nodiscard]] inline QUrl uriForAction(QStringView path,
                                           const ParamsMultiMap &queryValues) const;

    /**
     * Returns \c true if the last executed Action requested
     * that the processing be escaped.
     */
    bool detached() const noexcept;

    /**
     * The same as forward().
     *
     * When called with no arguments it escapes the processing chain entirely.
     *
     * \sa finalize()
     */
    void detach(Action *action = nullptr);

    /**
     * Detaches the processing chain telling the Engine that
     * the request is not finished yet.
     *
     * The scoped ASync class can make handling async requests easier.
     *
     * It’s often useful to call async API’s. While convenient, the use of QEventLoop
     * will only work for the first request or lead to a crash due stacking of calls.
     *
     * This method, tells the Engine that this request is not finished yet, making
     * it return to the event loop to process other requests or the task that was
     * created prior to calling this.
     *
     * Once done, call attachAsync() in order to process the remaining of the action chain.
     */
    void detachAsync() noexcept;

    /**
     * Reattaches to the processing chain in order to process the remaining of the action chaing.
     * Call this after you called detachAsync().
     *
     * The scoped ASync class can make handlying async requests easier.
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
     * This calls Component::execute().
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
     * This calls another action, by its private name.
     *
     * Keep in mind that the End() method used is that of the caller action.
     * So a c->detach() inside a forwarded action would run the End() method from
     * the original action requested.
     */
    bool forward(QStringView action);

    /**
     * Gets an \a action in a given namespace \a ns.
     */
    [[nodiscard]] Action *getAction(QStringView action, QStringView ns = {}) const;

    /**
     * Gets all actions of a given \a action name in a namespace \a ns and all parent namespaces.
     */
    [[nodiscard]] QVector<Action *> getActions(QStringView action, QStringView ns = {}) const;

    /**
     * Returns all registered plugins.
     */
    [[nodiscard]] QVector<Plugin *> plugins() const;

    /**
     * Returns the registered plugin that casts to the template type \a T.
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
     * Execute an action. Errors are available via errors().
     */
    bool execute(Component *code);

    /**
     * Returns the current locale to be used when processing a \ref plugins-view
     * or translating user messages.
     *
     * If not explicity set by setLocale() it will use the default locale set via
     * Application::setDefaultLocale().
     *
     * \sa \ref translations
     */
    [[nodiscard]] QLocale locale() const noexcept;

    /**
     * Defines the current locale to be used when processing a \ref plugins-view
     * or translating user messages.
     *
     * Setting a locale on a web application can be done in many ways,
     * so it’s up to the developer to decide which one to use.
     *
     * For example it’s possible to try to guess the user locale with
     * the request header Accept-Language, or use the chained dispatcher to first
     * match the locale as in "example.com/pt-br/some_action", or store
     * the locale into a cookie or session.
     *
     * Be sure to set it as soon as possible so that all content can be properly localized.
     *
     * \sa LangSelect
     * \sa \ref translations
     */
    void setLocale(const QLocale &locale);

    /**
     * Returns a configuration value for \a key. If \a key is not found, \a defaultValue will
     * be returned.
     *
     * This calls Application::config(), so it will only return entries from the \c %Cutelyst
     * configuration section.
     *
     * \sa \ref configuration
     */
    [[nodiscard]] QVariant config(const QString &key, const QVariant &defaultValue = {}) const;

    /**
     * Returns a configuration mapping for all configuration entries read from the \c %Cutelyst
     * configuration section.
     *
     * \sa \ref configuration
     */
    [[nodiscard]] QVariantMap config() const noexcept;

    /**
     * Translates the \a sourceText for the given \a context into the language defined by locale().
     *
     * See Application::addTranslator() for information about installation of translators.
     * Internally this function will use QTranslator::translate().
     *
     * \code{.cpp}
     * void MyController::index(Context *c)
     * {
     *      c->res()->setBody(c->translate("MyController", "You are on the index page."));
     * }
     * \endcode
     *
     * \sa \ref translations
     */
    [[nodiscard]] QString translate(const char *context,
                                    const char *sourceText,
                                    const char *disambiguation = nullptr,
                                    int n                      = -1) const;
    /**
     * Finds and returns a translated string.
     *
     * Returns a translated string identified by \a id. If no matching string is found,
     * the \a id itself is returned. This can be used similar to Qt’s global %qtTrId()
     * function.
     *
     * If \a n >= 0, all occurences of %%n in the resulting string are replaced with a
     * decimal representation of \a n. In addition, appending \a n’s value, the translation
     * may vary.
     *
     * Meta data and comments can be passed as documented for QObject::tr(). In addition,
     * it is possible to supply a source string template like that:
     *
     * <pre>//% &lt;C string&gt;</pre>
     *
     * or
     *
     * <pre>\begincomment% &lt;C string&gt; \endcomment</pre>
     *
     * Example:
     * \code{.cpp}
     * void MyController::index(Context *c)
     * {
     *      //% "%n fooish bar(s) found.\n"
     *      //% "Do you want to continue?"
     *      c->res()->setBody(c->qtTrId("my-app-translation-id", n));
     * }
     * \endcode
     *
     * Creating QM files suitable for use with this function requires passing the \c -idbased
     * option to the \c lrelease tool.
     *
     * \sa \ref translations
     *
     * \since Cutelyst 3.9.0
     */
    [[nodiscard]] inline QString qtTrId(const char *id, int n = -1) const;

public Q_SLOTS:
    /**
     * Finalize the request right away. This is automatically called
     * at the end of the actions chain.
     */
    void finalize();

protected:
    /**
     * Constructs a new %Context object using private implementation.
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

inline QUrl Context::uriForAction(QStringView path, const ParamsMultiMap &queryValues) const
{
    return uriForAction(path, QStringList(), QStringList(), queryValues);
}

inline QString Context::qtTrId(const char *id, int n) const
{
    return translate(nullptr, id, nullptr, n);
}

} // namespace Cutelyst

Q_DECLARE_METATYPE(Cutelyst::Context *)
