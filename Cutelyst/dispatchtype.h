/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_export.h>

#include <QtCore/qobject.h>
#include <QtCore/qstringlist.h>

namespace Cutelyst {

class Context;
class Action;
class Request;
/**
 * @ingroup core
 * @class Cutelyst::DispatchType dispatchtype.h Cutelyst/DispatchType
 * @brief Abstract class to described a dispatch type.
 *
 * This abstract class can be used to describe a dispatch type.
 */
class CUTELYST_EXPORT DispatchType : public QObject
{
    Q_OBJECT
public:
    /** This enum is used to describe the kind of a match  */
    enum MatchType {
        NoMatch = 0,
        PartialMatch,
        ExactMatch,
    };
    Q_ENUM(MatchType)

    /**
     * Construct a new %DispatchType object with the given \a parent.
     */
    explicit DispatchType(QObject *parent = nullptr);

    /**
     * Destroys the %DispatchType object.
     */
    virtual ~DispatchType();

    /**
     * Lists the registered actions.
     * Has to be implemented by subclasses.
     */
    virtual QByteArray list() const = 0;

    /**
     * Returns the MatchType for the given \a path and \a args.
     * Has to be implemented by subclasses.
     */
    [[nodiscard]] virtual MatchType
        match(Context *c, QStringView path, const QStringList &args) const = 0;

    /**
     * Returns an uri for an \a action with given \a captures.
     * Has to be implemented by subclasses.
     */
    [[nodiscard]] virtual QString uriForAction(Action *action,
                                               const QStringList &captures) const = 0;

    /**
     * Expand the \a action to a list of actions which is used in chained.
     * The default implementation does nothing and returns a \c nullptr.
     */
    [[nodiscard]] virtual Action *expandAction(const Context *c, Action *action) const;

    /**
     * Register an \a action and return \c true on success.
     * The default implementation does nothing and returns always \c true.
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
     * Returns \c true if the dispatch type has low precedence
     * when the precedence is the same the Class name is used
     * to sort them.
     *
     * The default implementation return \c false.
     */
    virtual bool isLowPrecedence() const;

protected:
    friend class Dispatcher;
    friend class Application;

    /**
     * Sets the matched \a action to the Context \a c.
     */
    void setupMatchedAction(Context *c, Action *action) const;
};

} // namespace Cutelyst
