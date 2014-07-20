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
#include <QStringList>

namespace Cutelyst {

namespace Plugin {
class AbstractPlugin;
}

class Action;
class Application;
class Engine;
class Request;
class Response;
class Dispatcher;
class Controller;
class ContextPrivate;
class Context : public QObject
{
    Q_OBJECT
public:
    Context(ContextPrivate *priv);
    ~Context();

    bool error() const;
    void error(const QString &error);
    QStringList errors() const;

    /**
     * Contains the return value of the last executed action.
     */
    bool state() const;
    void setState(bool state);
    QStringList args() const;
    QString uriPrefix() const;
    Engine *engine() const;

    Application *app() const;

    Response *response() const;
    Response *res() const;

    /**
     * Returns a pointer to the current action
     */
    Q_PROPERTY(Action* action READ action)
    Action *action() const;

    Q_PROPERTY(QString actionName READ actionName)
    QByteArray actionName() const;

    /**
     * Returns the namespace of the current action.
     * i.e. the URI prefix corresponding to the controller
     * of the current action. For example:
     * // a class named FooBar which inherits Controller
     * c->ns(); // returns 'foo/bar'
     */
    Q_PROPERTY(QString ns READ ns)
    QByteArray ns() const;

    /**
     * Returns the current Request object containing
     * information about the client request \sa Request
     */
    Request *request() const;

    /**
     * Short for the method above
     */
    Request *req() const;
    Dispatcher *dispatcher() const;

    Q_PROPERTY(QString controllerName READ controllerName)
    QString controllerName() const;

    Controller *controller(const QString &name = QString()) const;
    QString match() const;

    QVariantHash &stash();

    QByteArray uriFor(const QByteArray &path, const QStringList &args = QStringList());

    inline bool dispatch();
    bool detached() const;
    void detach();

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

Q_SIGNALS:
    void beforePrepareAction(bool *skipMethod);
    void beforeDispatch();
    void afterDispatch();

protected:
    void handleRequest();
    inline void prepareAction();
    void finalizeHeaders();
    inline void finalizeCookies();
    inline void finalizeBody();
    inline void finalizeError();
    int finalize();

    QVariant pluginProperty(Plugin::AbstractPlugin * const plugin, const QString &key, const QVariant &defaultValue = QVariant()) const;
    void setPluginProperty(Plugin::AbstractPlugin *plugin, const QString &name, const QVariant &value);

    friend class Application;
    friend class DispatchType;
    friend class Plugin::AbstractPlugin;
    ContextPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Context)
};

}

#endif // CUTELYST_CONTEXT_H
