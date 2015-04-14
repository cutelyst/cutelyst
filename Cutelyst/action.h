/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_ACTION_H
#define CUTELYST_ACTION_H

#include <QStringList>
#include <QMetaMethod>

#include <Cutelyst/Component>

namespace Cutelyst {

class Context;
class Controller;
class Dispatcher;
class ActionPrivate;
class Action : public Component
{
    Q_OBJECT
public:
    explicit Action(QObject *parent = 0);
    virtual ~Action();

    virtual Modifiers modifiers() const Q_DECL_OVERRIDE;

    /**
     * Returns the attributes that are set for this action,
     * like Local, Path, Private and so on. This determines
     * how the action is dispatched to.
     */
    QMap<QString, QString> attributes() const;

    void setAttributes(const QMap<QString, QString> &attributes);

    /**
     * @return Returns the name of the component where this action is defined
     */
    QString className() const;

    /**
     * @return Returns the controller where this action is defined
     */
    Controller* controller() const;

    /**
     * Dispatch this action against a context
     */
    virtual bool dispatch(Context *c);

    /**
     * @brief Check Args attribute, and makes sure number of
     * args matches the setting. Always returns true if Args is omitted.
     */
    bool match(int numberOfArgs) const;

    /**
     * @brief Can be implemented by action class
     * and action role authors. If the method exists,
     * then it will be called with the request context
     * and an array reference of the captures for this action.
     *
     * @return Returning true from this method causes the chain
     * match to continue, returning makes the chain not match
     * (and alternate, less preferred chains will be attempted).
     */
    bool matchCaptures(int numberOfCaptures) const;

    /**
     * Returns the private namespace this action lives in.
     */
    QString ns() const;

    /**
     * @brief numberOfArgs
     * @return Returns the number of args this action expects.
     * This is 0 if the action doesn't take any arguments and
     * undef if it will take any number of arguments.
     */
    qint8 numberOfArgs() const;

    /**
     * @brief numberOfCaptures
     * @return Returns the number of captures this action
     * expects for Chained actions.
     */
    qint8 numberOfCaptures() const;

protected:
    ActionPrivate *d_ptr;
    friend class Dispatcher;
    friend class ControllerPrivate;

    /**
     * Execute this action against
     */
    virtual bool doExecute(Context *c) Q_DECL_OVERRIDE;

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

typedef QList<Action*> ActionList;

}

#endif // CUTELYST_ACTION_H
