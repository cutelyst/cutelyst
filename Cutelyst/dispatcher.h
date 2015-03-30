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

#ifndef CUTELYST_DISPATCHER_H
#define CUTELYST_DISPATCHER_H

#include <QObject>
#include <QHash>
#include <QStringList>

#include <Cutelyst/Action>

namespace Cutelyst {

class Context;
class Controller;
class DispatchType;
class DispatcherPrivate;
class Dispatcher : public QObject
{
    Q_OBJECT
public:
    explicit Dispatcher(QObject *parent = 0);
    ~Dispatcher();

    /**
     * Returns a named action from a given namespace.
     */
    Action *getAction(const QString &name, const QString &nameSpace = QByteArray()) const;

    /**
     * Returns the named action by its full private path.
     */
    Action* getActionByPath(const QString &path) const;

    /**
     * Returns a list of actions that match \pa name on
     * the desired namespace \pa nameSpace
     */
    ActionList getActions(const QString &name, const QString &nameSpace) const;

    /**
     * Returns a hash of registered controllers
     */
    QHash<QString, Controller *> controllers() const;

    /**
     * Takes a Catalyst::Action object and action parameters and returns a URI
     * part such that if $c->req->path were this URI part, this action would be
     * dispatched to with $c->req->captures set to the supplied arrayref.
     *
     * If the action object is not available for external dispatch or the dispatcher
     * cannot determine an appropriate URI, this method will return a null byte array.
     */
    QString uriForAction(Action *action, const QStringList &captures) const;

    QList<DispatchType *> dispatchers() const;

protected:
    void setupActions(const QList<Controller *> &controllers);

    /**
     * Delegate the dispatch to the action that matched the url, or return a
     * message about unknown resource
     */
    bool dispatch(Context *ctx);

    bool forward(Context *ctx, Code *component);
    bool forward(Context *ctx, const QString &opname);
    void prepareAction(Context *ctx);

    /**
     * Returns a list of actions on the desired namespace \pa ns
     */
//    ActionList getActionsNs(const QByteArray &ns) const;

    void registerDispatchType(DispatchType *dispatchType);

protected:
    friend class Application;
    friend class Context;
    friend class Controller;
    DispatcherPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Dispatcher)
};

}

#endif // CUTELYST_DISPATCHER_H
