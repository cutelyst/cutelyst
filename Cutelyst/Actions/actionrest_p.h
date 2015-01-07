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

#ifndef ACTIONREST_P_H
#define ACTIONREST_P_H

#include "actionrest.h"

namespace Cutelyst {

class ActionRESTPrivate
{
    Q_DECLARE_PUBLIC(ActionREST)
public:
    bool dispatchRestMethod(Context *ctx, const QString &restMethod) const;
    bool returnOptions(Context *ctx, const QString &methodName) const;
    bool returnNotImplemented(Context *ctx, const QString &methodName) const;
    QString getAllowedMethods(Controller *controller, const QString &methodName) const;

    ActionREST *q_ptr;
};

}

#endif // ACTIONREST_P_H
