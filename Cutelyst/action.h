/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_ACTION_H
#define CUTELYST_ACTION_H

#include <Cutelyst/component.h>
#include <Cutelyst/context.h>
#include <Cutelyst/cutelyst_global.h>

#include <QtCore/QMetaMethod>
#include <QtCore/QStringList>

namespace Cutelyst {

class Controller;
class Dispatcher;
class ActionPrivate;
/**
 * \ingroup core
 * \class Action action.h Cutelyst/Action
 * \brief This class represents a %Cutelyst %Action.
 *
 * This class represents a %Cutelyst Action.
 *
 * It encapsulates a Controller method that was defined with <tt>C_ATTR()</tt> macro, it’s usually
 * automatically created by Controller using it’s introspection information. This class allows
 * %Cutelyst to call a Controller method.
 *
 * You can access the object for the currently dispatched action via
 * \link Context::action() c->action()\endlink. See the Dispatcher for more information on how
 * actions are dispatched.
 */
class CUTELYST_LIBRARY Action : public Component
{
    Q_OBJECT
public:
    /**
     * Constructs an %Action object with the given \a parent.
     */
    explicit Action(QObject *parent = nullptr);

    /**
     * Destroys the %Action object.
     */
    virtual ~Action() override = default;

    /**
     * Always returns Component::OnlyExecute.
     */
    virtual Modifiers modifiers() const override;

    /**
     * Returns the attributes that are set for this action,
     * like \c Local, \c Path, \c Private and so on. This determines
     * how the action is dispatched to.
     */
    [[nodiscard]] ParamsMultiMap attributes() const noexcept;

    /**
     * Returns the value attribute by it’s \a name, if not found
     * \a defaultValue is returned.
     *
     * Attributes can be defined using the @c C_ATTR macro on Controller’s
     * method declaration.
     */
    [[nodiscard]] QString attribute(const QString &name, const QString &defaultValue = {}) const;

    /**
     * Defines the action’s attibutes that were defined using the @c C_ATTR macro on Controller’s
     * method declaration.
     */
    void setAttributes(const ParamsMultiMap &attributes);

    /**
     * Returns the name of the component where this action is defined.
     */
    [[nodiscard]] QString className() const noexcept;

    /**
     * Returns the controller where this action is defined.
     */
    [[nodiscard]] Controller *controller() const noexcept;

    /**
     * Dispatch this action against context \a c.
     */
    inline bool dispatch(Context *c) { return c->execute(this); }

    /**
     * Check \c Args attribute, and makes sure number of
     * args matches the setting. Always returns \c true if \c Args is omitted.
     */
    [[nodiscard]] virtual bool match(int numberOfArgs) const noexcept;

    /**
     * Can be implemented by action class
     * and action role authors. If the method exists,
     * then it will be called with the request context
     * and an array reference of the captures for this action.
     *
     * Returning \c true from this method causes the chain
     * match to continue, returning \c false makes the chain not match
     * (and alternate, less preferred chains will be attempted).
     */
    [[nodiscard]] virtual bool matchCaptures(int numberOfCaptures) const noexcept;

    /**
     * Returns the private namespace this action lives in.
     */
    [[nodiscard]] QString ns() const noexcept;

    /**
     * Returns the number of args this action expects.
     * This is \c 0 if the action doesn’t take any arguments and
     * undef if it will take any number of arguments.
     */
    [[nodiscard]] virtual qint8 numberOfArgs() const;

    /**
     * Returns the number of captures this action
     * expects for Chained actions.
     */
    [[nodiscard]] virtual qint8 numberOfCaptures() const;

protected:
    friend class Dispatcher;
    friend class ControllerPrivate;

    /**
     * A derived class using pimpl should call this constructor, to reduce the number of memory
     * allocations.
     */
    explicit Action(ActionPrivate *ptr, QObject *parent = nullptr);

    /**
     * Execute this action against context \a c.
     */
    bool doExecute(Context *c) override;

    /**
     * Sets the \a method to be invoked by this action.
     */
    void setMethod(const QMetaMethod &method);

    /**
     * Sets the \a controller which this action belongs to.
     */
    void setController(Controller *controller);

    /**
     * Called by dispatcher to setup the action.
     */
    void setupAction(const QVariantHash &args, Application *app);

private:
    Q_DECLARE_PRIVATE(Action)
};

/** Defines a list (vector) of Action pointers */
typedef QVector<Action *> ActionList;

} // namespace Cutelyst

#endif // CUTELYST_ACTION_H
