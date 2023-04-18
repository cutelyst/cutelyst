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
/*! \class Action action.h Cutelyst/Action
 * \brief This class represents a %Cutelyst %Action.
 *
 * This class represents a Cutelyst Action.
 *
 * It encapsulates a Controller method that was defined with
 * C_ATTR macro, it's usually automatically created by Cutelyst::Controller
 * using it's introspections information, this class allows Cutelyst
 * to call a Controller method.
 *
 * You can access the object for the currently dispatched
 * action via c->action(). See the Cutelyst::Dispatcher for
 * more information on how actions are dispatched.
 */
class CUTELYST_LIBRARY Action : public Component
{
    Q_OBJECT
public:
    /**
     * Constructs a Action object with the given \p parent.
     */
    explicit Action(QObject *parent = nullptr);
    virtual ~Action() override = default;

    virtual Modifiers modifiers() const override;

    /**
     * Returns the attributes that are set for this action,
     * like Local, Path, Private and so on. This determines
     * how the action is dispatched to.
     */
    ParamsMultiMap attributes() const noexcept;

    /**
     * Returns the value attribute by it's name, if not found
     * dafault value is returned.
     *
     * Attributes can be defined using the C_ATTR macro on Controller's
     * method declaration.
     */
    QString attribute(const QString &name, const QString &defaultValue = {}) const;

    /**
     * Defines the Actions attibutes that were defined using the C_ATTR macro on Controller's
     * method declaration.
     */
    void setAttributes(const ParamsMultiMap &attributes);

    /**
     * Returns the name of the component where this action is defined
     */
    QString className() const;

    /**
     * Returns the controller where this action is defined
     */
    Controller *controller() const;

    /**
     * Dispatch this action against a context
     */
    inline bool dispatch(Context *c) { return c->execute(this); }

    /**
     * Check Args attribute, and makes sure number of
     * args matches the setting. Always returns true if Args is omitted.
     */
    virtual bool match(int numberOfArgs) const noexcept;

    /**
     * Can be implemented by action class
     * and action role authors. If the method exists,
     * then it will be called with the request context
     * and an array reference of the captures for this action.
     *
     * @return Returning true from this method causes the chain
     * match to continue, returning false makes the chain not match
     * (and alternate, less preferred chains will be attempted).
     */
    virtual bool matchCaptures(int numberOfCaptures) const noexcept;

    /**
     * Returns the private namespace this action lives in.
     */
    QString ns() const noexcept;

    /**
     * Returns the number of args this action expects.
     * This is 0 if the action doesn't take any arguments and
     * undef if it will take any number of arguments.
     */
    virtual qint8 numberOfArgs() const noexcept;

    /**
     * Returns the number of captures this action
     * expects for Chained actions.
     */
    virtual qint8 numberOfCaptures() const noexcept;

protected:
    friend class Dispatcher;
    friend class ControllerPrivate;

    /*!
     * A derived class using pimpl should call this constructor, to reduce the number of memory allocations
     */
    explicit Action(ActionPrivate *ptr, QObject *parent = nullptr);

    /**
     * Execute this action against
     */
    virtual bool doExecute(Context *c) override;

    /**
     * The method to be invoked by this Action
     */
    void setMethod(const QMetaMethod &method);

    /**
     * The controller which this action belongs to
     */
    void setController(Controller *controller);

    /**
     * Called by dispatcher to setup the Action
     */
    void setupAction(const QVariantHash &args, Application *app);

private:
    Q_DECLARE_PRIVATE(Action)
};

/** Defines a list (vector) of Action pointers */
typedef QVector<Action *> ActionList;

} // namespace Cutelyst

#endif // CUTELYST_ACTION_H
