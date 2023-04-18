/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ACTIONREST_P_H
#define ACTIONREST_P_H

#include "action_p.h"
#include "actionrest.h"

namespace Cutelyst {

class ActionRESTPrivate : ActionPrivate
{
    Q_DECLARE_PUBLIC(ActionREST)
public:
    explicit ActionRESTPrivate(ActionREST *q);
    bool dispatchRestMethod(Context *c, const QString &restMethod) const;
    bool returnOptions(Context *c, const QString &methodName) const;
    bool returnNotImplemented(Context *c, const QString &methodName) const;
    QString getAllowedMethods(Controller *controller, const QString &methodName) const;

    ActionREST *q_ptr;
};

} // namespace Cutelyst

#endif // ACTIONREST_P_H
