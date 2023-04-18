/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_COMPONENT_P_H
#define CUTELYST_COMPONENT_P_H

#include "component.h"

#include <QtCore/qstack.h>

namespace Cutelyst {

class ComponentPrivate
{
public:
    virtual ~ComponentPrivate() = default;

    QString name;
    QString reverse;
    QStack<Component *> beforeRoles;
    QStack<Component *> aroundRoles;
    QStack<Component *> afterRoles;
    QStack<Component *> roles;
    bool proccessRoles = false;
};

} // namespace Cutelyst

#endif // CUTELYST_COMPONENT_P_H
