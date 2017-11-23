/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef DISPATCHTYPE_H
#define DISPATCHTYPE_H

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class Context;
class Action;
class Request;
class CUTELYST_LIBRARY DispatchType : public QObject
{
    Q_OBJECT
public:
    /** This enum is used to describe the kind of a match  */
    enum MatchType {
        NoMatch = 0,
        PartialMatch,
        ExactMatch
    };
    Q_ENUM(MatchType)

    /**
     * Construct a DispatchType object
     */
    explicit DispatchType(QObject *parent = nullptr);
    virtual ~DispatchType();

    /**
     * @brief list the registered actions
     * To be implemented by subclasses
     */
    virtual QByteArray list() const = 0;

    /**
     * Return true if the dispatchType matches the given path
     */
    virtual MatchType match(Context *c, const QString &path, const QStringList &args) const = 0;

    /**
     * Returns an uri for an action
     */
    virtual QString uriForAction(Action *action, const QStringList &captures) const = 0;

    /**
     * Expand the action to a list of actions which is used in chained
     */
    virtual Action *expandAction(Context *c, Action *action) const;

    /**
     * @brief registerAction
     * @param action
     * @return
     */
    virtual bool registerAction(Action *action);

    /**
     * If false the dispatcher will be unregistered for
     * performance reasons. This method can be used to
     * prepare actions for dispatcher as in a Chain of
     * Actions there is no garantee of registering order.
     *
     * In the common case if the dispatcher has registered
     * any action, or in some special case that it doesn't need
     * actions it will return true.
     */
    virtual bool inUse() = 0;

    /**
     * Returns true if the dispatch type has low precedence
     * when the precedence is the same the Class name is used
     * to sort them.
     */
    virtual bool isLowPrecedence() const;

protected:
    friend class Dispatcher;
    friend class Application;

    /**
     * Sets the matched action to the Context
     */
    void setupMatchedAction(Context *c, Action *action) const;
};

}

#endif // DISPATCHTYPE_H
