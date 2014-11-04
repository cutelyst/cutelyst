/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#include <QObject>
#include <QHash>
#include <QVariant>
#include <QUrl>
#include <QStringList>
#include <QStack>

#include <Cutelyst/Request>

namespace Cutelyst {

namespace Plugin {
class AbstractPlugin;
}

class Action;
class Application;
class Engine;
class Response;
class Dispatcher;
class Controller;
class ContextPrivate;
class Context : public QObject
{
    Q_OBJECT
    Q_PROPERTY(Action* action READ action)
    Q_PROPERTY(QByteArray actionName READ actionName)
    Q_PROPERTY(QByteArray ns READ ns)
    Q_PROPERTY(Request *req READ request)
    Q_PROPERTY(Request *request READ request)
    Q_PROPERTY(QByteArray controllerName READ controllerName)
    Q_PROPERTY(bool state READ state)
public:
    Context(ContextPrivate *priv);
    virtual ~Context();

    bool error() const;
    void error(const QString &error);
    QStringList errors() const;

    /**
     * Contains the return value of the last executed action.
     */
    bool state() const;

    void setState(bool state);
    inline QStringList args() const
    { return request()->args(); }

    QString uriPrefix() const;
    Engine *engine() const;

    Application *app() const;

    Response *response() const;
    Response *res() const;

    /**
     * Returns a pointer to the current action
     */
    Action *action() const;

    /**
     * Returns the private name of the current action
     */
    QByteArray actionName() const;

    /**
     * Returns the namespace of the current action.
     * i.e. the URI prefix corresponding to the controller
     * of the current action. For example:
     * // a class named FooBar which inherits Controller
     * ctx->ns(); // returns 'foo/bar'
     */
    QByteArray ns() const;

    /**
     * Returns the current Request object containing
     * information about the client request Request
     */
    Request *request() const;

    /**
     * Short for request()
     */
    Request *req() const;

    Dispatcher *dispatcher() const;

    /**
     * The current controller name
     */
    QByteArray controllerName() const;

    Controller *controller(const QByteArray &name = QByteArray()) const;

    QVariantHash &stash();

    /**
     * Returns the internal execution stack (actions that are currently executing).
     */
    QStack<Action *> stack() const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     *
     * The first argument is taken as a public URI path relative
     * ctx->ns (if it doesn't begin with a forward slash) or
     * relative to the application root (if it does). It is then merged with
     * ctx->request()->base() any \p args are appended as additional path
     * components; and any queryValues> are appended as "?foo=bar" parameters.
     */
    QUrl uriFor(const QByteArray &path = QByteArray(),
                const QStringList &args = QStringList(),
                const QMultiHash<QString, QString> &queryValues = QMultiHash<QString, QString>()) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     *
     * The first argument is taken as a public URI path relative
     * ctx->ns (if it doesn't begin with a forward slash) or
     * relative to the application root (if it does). It is then merged with
     * ctx->request()->base() and any queryValues> are appended as "?foo=bar" parameters.
     */
    inline QUrl uriForNoArgs(const QByteArray &path,
                             const QMultiHash<QString, QString> &queryValues) const
    { return uriFor(path, QStringList(), queryValues); }

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     *
     * If no arguments are provided, the URI for the current action is returned.
     * To return the current action and also provide \p args, use
     * ctx->uriFor(ctx->action(), args).
     */
    QUrl uriFor(Action *action,
                const QStringList &args = QStringList(),
                const QMultiHash<QString, QString> &queryValues = QMultiHash<QString, QString>()) const;

    /**
     * Constructs an absolute QUrl object based on the application root, the
     * provided path, and the additional arguments and query parameters provided.
     * When used as a string, provides a textual URI.
     */
    inline QUrl uriForNoArgs(Action *action,
                             const QMultiHash<QString, QString> &queryValues) const
    { return uriFor(action, QStringList(), queryValues); }

    /**
     * A private path to the Cutelyst action you want to create a URI for.
     *
     * This is a shortcut for calling ctx->dispatcher()->getActionByPath(path)
     * and passing the resulting action and the remaining arguments to ctx->uri_for.
     *
     * Note that although the path looks like a URI that dispatches to the wanted action,
     * it is not a URI, but an internal path to that action.
     *
     * For example, if the action looks like:
     * \code{.h}
     * class Users : public Cutelyst::Controller
     * {
     * public:
     *   C_ATTR(lst, :Path(the-list)
     *   void lst(Context *ctx);
     * };
     * \endcode
     *
     * You can use:
     * ctx->uriForAction('/users/lst');
     * and it will create the URI /users/the-list.
     */
    QUrl uriForAction(const QByteArray &path,
                      const QStringList &args = QStringList(),
                      const QMultiHash<QString, QString> &queryValues = QMultiHash<QString, QString>()) const;

    /**
     * A convenience method for the uriForAction() without the arguments parameter
     */
    inline QUrl uriForActionNoArgs(const QByteArray &path,
                                   const QMultiHash<QString, QString> &queryValues) const
    { return uriForAction(path, QStringList(), queryValues); }

    bool detached() const;

    /**
     * The same as forward(action), but doesn't return to the previous
     * action when processing is finished.
     * When called with no arguments it escapes the processing chain entirely.
     */
    void detach(Action *action = 0);

    bool forward(Action *action, const QStringList &arguments = QStringList());
    bool forward(const QByteArray &action, const QStringList &arguments = QStringList());
    Action *getAction(const QByteArray &action, const QByteArray &ns = QByteArray());
    QList<Action*> getActions(const QByteArray &action, const QByteArray &ns = QByteArray());

    bool registerPlugin(Cutelyst::Plugin::AbstractPlugin *plugin, bool takeOwnership = true);
    QList<Plugin::AbstractPlugin *> plugins();

    template <typename T>
    T plugin()
    {
        Q_FOREACH (Plugin::AbstractPlugin *plugin, plugins()) {
            if (qobject_cast<T>(plugin)) {
                return qobject_cast<T>(plugin);
            }
        }
        return 0;
    }

    /**
     * Execute an action. Errors are available via error().
     */
    bool execute(Action *action);

Q_SIGNALS:
    void beforePrepareAction(bool *skipMethod);
    void beforeDispatch();
    void afterDispatch();

protected:
    QVariant pluginProperty(Plugin::AbstractPlugin * const plugin, const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setPluginProperty(Plugin::AbstractPlugin *plugin, const QString &name, const QVariant &value);

    friend class Application;
    friend class Action;
    friend class DispatchType;
    friend class Plugin::AbstractPlugin;
    ContextPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Context)
};

}

Q_DECLARE_METATYPE(Cutelyst::Context *)

#endif // CUTELYST_CONTEXT_H
