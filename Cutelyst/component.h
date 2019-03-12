/*
 * Copyright (C) 2014-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYST_COMPONENT_H
#define CUTELYST_COMPONENT_H

#include <QtCore/qobject.h>

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class Application;
class Context;
class Controller;
class Dispatcher;
class ComponentPrivate;

/*! \class Component component.h Cutelyst/Component
 * @brief The %Cutelyst %Component base class
 *
 * This is the base class of a Cutelyst component
 */
class CUTELYST_LIBRARY Component : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Component)
    Q_FLAGS(Modifiers)
public:
    /**  This value defines which kind of modifiers should be executed */
    enum Modifier {
        None          = 0 << 1,
        OnlyExecute   = 1 << 1,
        BeforeExecute = 2 << 1,
        AroundExecute = 3 << 1,
        AfterExecute  = 4 << 1,
    };
    Q_ENUM(Modifier)
    Q_DECLARE_FLAGS(Modifiers, Modifier)

    /**
     * This is the base class for many Cutelyst objects,
     * prividing access to name and reverse for actions,
     * and modifiers to customize execution.
     */
    explicit Component(QObject *parent = nullptr);
    virtual ~Component();

    /**
     * Reimplement to return custom Modifiers, default is None
     */
    virtual Modifiers modifiers() const;

    /**
     * Returns the sub name of this Component.
     */
    QString name() const;

    /**
     * Defines the sub name of this Component.
     */
    void setName(const QString &name);

    /**
     * Returns the private name of this component.
     */
    QString reverse() const;

    /**
     * Defines the private name of this Component.
     */
    void setReverse(const QString &reverse);

    /**
     * A Does class is always attached to an action,
     * if this method returns false the application
     * will fail to start. Often useful if the user
     * misconfigured the settings
     */
    virtual bool init(Application *application, const QVariantHash &args);

    /**
     * Executes this component agains the Context
     */
    bool execute(Context *c);

protected:
    /*!
     * A derived class using pimpl should call this constructor, to reduce the number of memory allocations
     */
    explicit Component(ComponentPrivate *d, QObject *parent = nullptr);

    /**
     * Reimplement this if you want to do processing before doExecute
     */
    virtual bool beforeExecute(Context *c);

    /**
     * Reimplement this if you want to do processing around doExecute,
     * you must call doExecute yourself then
     */
    virtual bool aroundExecute(Context *c, QStack<Component *> stack);

    /**
     * Reimplement this if you want to do processing after doExecute
     */
    virtual bool afterExecute(Context *c);

    /**
     * Reimplement this for the main processing
     */
    virtual bool doExecute(Context *c);

    /**
     * Call this to install before, around and after roles
     */
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
    ComponentPrivate *d_ptr; //!< we cannot inherit from QObjectPrivate and therefore need our own d_ptr
};

}

#endif // CUTELYST_COMPONENT_H
