/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef DISPATCHTYPE_H
#define DISPATCHTYPE_H

#include <Cutelyst/cutelyst_global.h>

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

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
    virtual Action *expandAction(const Context *c, Action *action) const;

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

} // namespace Cutelyst

#endif // DISPATCHTYPE_H
