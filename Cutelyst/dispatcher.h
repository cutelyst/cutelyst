/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/action.h>
#include <Cutelyst/cutelyst_global.h>

#include <QtCore/qhash.h>
#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

namespace Cutelyst {

class Context;
class Controller;
class DispatchType;
class DispatcherPrivate;

/*! \class Dispatcher dispatcher.h Cutelyst/Dispatcher
 * @brief The %Cutelyst %Dispatcher
 *
 * This class is resposible for finding an Action for new Requests and invoking it.
 */
class CUTELYST_LIBRARY Dispatcher : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a Dispatcher object with the given \p parent.
     */
    Dispatcher(QObject *parent = nullptr);
    ~Dispatcher();

    /**
     * Returns a named action from a given namespace.
     */
    Action *getAction(QStringView name, const QString &nameSpace = {}) const;

    /**
     * Returns the named action by its full private path.
     */
    Action *getActionByPath(QStringView path) const;

    /**
     * Returns a list of actions that match \p name on
     * the desired namespace \p nameSpace
     */
    ActionList getActions(QStringView name, const QString &nameSpace) const;

    /**
     * Returns a Controller given it's name
     */
    Controller *controller(QStringView name) const;

    /**
     * Returns a list of registered controllers
     */
    QList<Controller *> controllers() const;

    /**
     * Takes a Catalyst::Action object and action parameters and returns a URI
     * part such that if $c->req->path were this URI part, this action would be
     * dispatched to with $c->req->captures set to the supplied arrayref.
     *
     * If the action object is not available for external dispatch or the dispatcher
     * cannot determine an appropriate URI, this method will return a null byte array.
     */
    QString uriForAction(Action *action, const QStringList &captures) const;

    /**
     * Expand an action into a full representation of the dispatch. mostly useful for chained where
     * the returned Action will be of ActionChain type, other actions will just return a single
     * action.
     */
    Action *expandAction(const Context *c, Action *action) const;

    /**
     * Returns a list of all dispatchers currently in use, if the dispatcher doesn't successfuly
     * register an Action it's removed from the list.
     */
    QVector<DispatchType *> dispatchers() const;

protected:
    /**
     * Used by Application to register all Controllers Actions into the list of DispatchType
     */
    void setupActions(const QVector<Controller *> &controllers,
                      const QVector<DispatchType *> &dispatchers,
                      bool printActions);

    /**
     * Delegate the dispatch to the action that matched the url, or return a
     * message about unknown resource
     */
    bool dispatch(Context *c);

    /**
     * Used by Application to forward execution to the following Component
     */
    bool forward(Context *c, Component *component);

    /**
     * Used by Application to forward execution to \p opname that is resolved to an Action
     */
    bool forward(Context *c, QStringView opname);

    /**
     * Used by Application to find a matching action for the current Context
     */
    void prepareAction(Context *c);

protected:
    friend class Application;
    friend class Context;
    friend class Controller;
    DispatcherPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Dispatcher)
};

} // namespace Cutelyst
