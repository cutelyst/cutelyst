/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef ACTIONREST_H
#define ACTIONREST_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/action.h>
#include <Cutelyst/componentfactory.h>

namespace Cutelyst {

class ActionRESTPrivate;
class CUTELYST_PLUGIN_ACTION_REST_EXPORT ActionREST : public Action
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ActionREST)
public:
    /**
     * Constructs a new ActionREST object with the given parent.
     */
    explicit ActionREST(QObject *parent = nullptr);

protected:
    bool doExecute(Context *c) override;
};

class ActionRESTFactory : public QObject, public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    virtual Component *createComponent(QObject *parent) override { return new ActionREST(parent); }
};

}

#endif // ACTIONREST_H
