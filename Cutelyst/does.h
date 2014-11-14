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

#ifndef DOES_H
#define DOES_H

#include <QObject>

#include <Cutelyst/Context>

namespace Cutelyst {

class Context;
class DoesPrivate;
class Does : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Does)
    Q_ENUMS(Modifier)
    Q_FLAGS(Modifiers)
public:
    class DoesCode {
        friend class Does;
    protected:
        Does *execute;
        QStack<Does *> stack;
    };

    enum Modifier {
        OnlyExecute   = 0 << 1,
        BeforeExecute = 1 << 1,
        AroundExecute = 2 << 1,
        AfterExecute  = 3 << 1,
    };
    Q_DECLARE_FLAGS(Modifiers, Modifier)

    Does();
    virtual ~Does();

    virtual Modifiers modifiers() const = 0;

    /**
     * A Does class is always attached to an action,
     * if this method returns false the application
     * will fail to start. Often useful if the user
     * misconfigured the settings
     */
    virtual bool init(Application *application, const QVariantHash &args);

    bool execute(Context *ctx);

protected:
    virtual bool beforeExecute(Context *ctx);

    virtual bool aroundExecute(Context *ctx, DoesCode code);

    virtual bool afterExecute(Context *ctx);

    virtual bool doExecute(Context *ctx);

    void applyRoles(const QStack<Does *> &roles);

    /**
     * Called by dispatcher once it's done preparing actions
     *
     * Subclasses might want to implement this to cache special
     * actions, such as special methods for REST actions
     */
    virtual bool dispatcherReady(const Dispatcher *dispatch, Controller *controller);

protected:
    friend class Controller;
    DoesPrivate *d_ptr;
};

}

#endif // DOES_H
