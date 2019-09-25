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
#ifndef ACTIONREST_P_H
#define ACTIONREST_P_H

#include "actionrest.h"
#include "action_p.h"

namespace Cutelyst {

class ActionRESTPrivate : ActionPrivate
{
    Q_DECLARE_PUBLIC(ActionREST)
public:
    explicit ActionRESTPrivate(ActionREST* q);
    bool dispatchRestMethod(Context *c, const QString &restMethod) const;
    bool returnOptions(Context *c, const QString &methodName) const;
    bool returnNotImplemented(Context *c, const QString &methodName) const;
    QString getAllowedMethods(Controller *controller, const QString &methodName) const;

    ActionREST *q_ptr;
};

}

#endif // ACTIONREST_P_H
