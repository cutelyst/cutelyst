/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef COMPONENTFACTORY_H
#define COMPONENTFACTORY_H

#include <Cutelyst/Context>

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

} // namespace Cutelyst

#define ComponentFactory_iid "org.cutelyst.ComponentFactory"
Q_DECLARE_INTERFACE(Cutelyst::ComponentFactory, ComponentFactory_iid)

#endif // COMPONENTFACTORY_H
