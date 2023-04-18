/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ROLEACL_P_H
#define ROLEACL_P_H

#include "component_p.h"
#include "roleacl.h"

namespace Cutelyst {

class RoleACLPrivate : public ComponentPrivate
{
public:
    QStringList requiresRole;
    QStringList allowedRole;
    QString aclDetachTo;
    QString actionReverse;
    Action *detachTo;
};

} // namespace Cutelyst

#endif // ROLEACL_P_H
