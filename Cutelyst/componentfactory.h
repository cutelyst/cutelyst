/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include <Context>
#include <QString>

namespace Cutelyst {

class ComponentFactory
{
public:
    virtual ~ComponentFactory() {}

    /**
     * Component plugins should reimplement this to get a new
     * instace of their component
     */
    virtual Component *createComponent(QObject *parent = nullptr) = 0;
};

}

#define ComponentFactory_iid "org.cutelyst.ComponentFactory"
Q_DECLARE_INTERFACE(Cutelyst::ComponentFactory, ComponentFactory_iid)

#endif // COMPONENTFACTORY_H
