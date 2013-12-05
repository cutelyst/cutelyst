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

#ifndef CUTELYST_ACTION_H
#define CUTELYST_ACTION_H

#include <QObject>
#include <QStringList>
#include <QMetaMethod>

namespace Cutelyst {

class Context;
class Controller;
class ActionPrivate;
class Action : public QObject
{
    Q_OBJECT
public:
    explicit Action(const QMetaMethod &method, Controller *parent = 0);
    ~Action();

    /**
     * @return The sub attributes that are set for this action,
     * like Local, Path, Private and so on. This determines
     * how the action is dispatched to.
     */
    QMultiHash<QString, QString> attributes() const;

    /**
     * @return Returns the name of the component where this action is defined
     */
    QString className() const;

    /**
     * @return Returns the controller where this action is defined
     */
    Controller* controller() const;

    /**
     * @brief dispatch Dispatch this action against a context
     */
    bool dispatch(Context *ctx);

    /**
     * @brief Check Args attribute, and makes sure number of
     * args matches the setting. Always returns true if Args is omitted.
     */
    bool match(Context *ctx) const;

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
    bool matchCaptures(Context *ctx) const;

    /**
     * @brief name
     * @return Returns the sub name of this action.
     */
    QString name() const;

    /**
     * @brief name
     * @return Returns the private name of this action.
     */
    QString privateName() const;

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
    quint8 numberOfArgs() const;

    /**
     * @brief numberOfCaptures
     * @return Returns the number of captures this action
     * expects for Chained actions.
     */
    quint8 numberOfCaptures() const;

    /**
     * @brief meta
     * @return Returns the meta information about this action.
     */
    QMetaMethod meta() const;

    bool isValid() const;

protected:
    ActionPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(Action)
};

typedef QList<Action*> ActionList;

}

#endif // CUTELYST_ACTION_H
