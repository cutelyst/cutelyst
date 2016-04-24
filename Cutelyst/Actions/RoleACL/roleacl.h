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

#ifndef ROLEACL_H
#define ROLEACL_H

#include <QtCore/QVariantHash>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/component.h>
#include <Cutelyst/context.h>
#include <Cutelyst/componentfactory.h>

namespace Cutelyst {

class RoleACLPrivate;
class CUTELYST_LIBRARY RoleACL : public Component
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RoleACL)
public:
    explicit RoleACL(QObject *parent = nullptr);
    virtual ~RoleACL();

    virtual Modifiers modifiers() const Q_DECL_OVERRIDE;

    virtual bool init(Application *application, const QVariantHash &args) Q_DECL_OVERRIDE;

    virtual bool aroundExecute(Context *c, QStack<Component *> stack) Q_DECL_OVERRIDE;

    bool canVisit(Context *c) const;

protected:
    virtual bool dispatcherReady(const Dispatcher *dispatcher, Controller *controller) Q_DECL_OVERRIDE;

    RoleACLPrivate *d_ptr;
};

class RoleACLFactory : public QObject, public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    Component *createComponent(QObject *parent) { return new RoleACL(parent); }
};


}

#endif // ROLEACL_H
