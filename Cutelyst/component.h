/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CUTELYST_COMPONENT_H
#define CUTELYST_COMPONENT_H

#include <QObject>

#include <Cutelyst/Context>

namespace Cutelyst {

class Context;
class ComponentPrivate;
class Component : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Component)
    Q_ENUMS(Modifier)
    Q_FLAGS(Modifiers)
public:
    enum Modifier {
        OnlyExecute   = 0 << 1,
        BeforeExecute = 1 << 1,
        AroundExecute = 2 << 1,
        AfterExecute  = 3 << 1,
    };
    Q_DECLARE_FLAGS(Modifiers, Modifier)

    explicit Component(QObject *parent = 0);
    virtual ~Component();

    virtual Modifiers modifiers() const = 0;

    /**
     * @brief name
     * @return Returns the sub name of this Component.
     */
    QString name() const;

    void setName(const QString &name);

    /**
     * @brief name
     * @return Returns the private name of this action.
     */
    QString reverse() const { return objectName(); }

    void setReverse(const QString &reverse)
    { return setObjectName(reverse); }

    /**
     * A Does class is always attached to an action,
     * if this method returns false the application
     * will fail to start. Often useful if the user
     * misconfigured the settings
     */
    virtual bool init(Application *application, const QVariantHash &args);

    bool execute(Context *c);

protected:
    virtual bool beforeExecute(Context *c);

    virtual bool aroundExecute(Context *c, QStack<Component *> stack);

    virtual bool afterExecute(Context *c);

    virtual bool doExecute(Context *c);

    void applyRoles(const QStack<Component *> &roles);

    /**
     * Called by dispatcher once it's done preparing actions
     *
     * Subclasses might want to implement this to cache special
     * actions, such as special methods for REST actions
     */
    virtual bool dispatcherReady(const Dispatcher *dispatch, Controller *controller);

protected:
    friend class Controller;
    ComponentPrivate *d_ptr;
};

}

#endif // CUTELYST_COMPONENT_H
