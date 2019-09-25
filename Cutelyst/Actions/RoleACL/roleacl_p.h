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
#ifndef ROLEACL_P_H
#define ROLEACL_P_H

#include "roleacl.h"
#include "component_p.h"

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

}

#endif // ROLEACL_P_H
