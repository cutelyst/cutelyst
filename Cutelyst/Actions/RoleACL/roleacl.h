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
#ifndef ROLEACL_H
#define ROLEACL_H

#include <QtCore/QVariantHash>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/component.h>
#include <Cutelyst/context.h>
#include <Cutelyst/componentfactory.h>

namespace Cutelyst {

class RoleACLPrivate;
class CUTELYST_PLUGIN_ACTION_ROLEACL_EXPORT RoleACL : public Component
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RoleACL)
public:
    /**
     * Constructs a new role ACL object with the given parent.
     */
    explicit RoleACL(QObject *parent = nullptr);

    /**
     * Reimplemented from Component::modifiers().
     */
    virtual Modifiers modifiers() const override;

    /**
     * Reimplemented from Component::init().
     */
    virtual bool init(Application *application, const QVariantHash &args) override;

    /**
     * Reimplemented from Component::aroundExecute().
     */
    virtual bool aroundExecute(Context *c, QStack<Component *> stack) override;

    /**
     * Returns true if the action can be visited by the context c.
     */
    bool canVisit(Context *c) const;

protected:
    /**
     * Reimplemented from Component::dispatcherReady().
     */
    virtual bool dispatcherReady(const Dispatcher *dispatcher, Controller *controller) override;
};

class RoleACLFactory : public QObject, public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    virtual Component *createComponent(QObject *parent) override { return new RoleACL(parent); }
};


}

#endif // ROLEACL_H
