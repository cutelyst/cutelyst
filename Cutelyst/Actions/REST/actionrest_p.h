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
    Action *getRestAction(Context *c, const QByteArray &httpMethod) const;
    void returnOptions(Context *c, const QString &methodName) const;
    void returnNotImplemented(Context *c, const QString &methodName) const;
    QByteArray getAllowedMethods(const Controller *controller, const QString &methodName) const;

    ActionREST *q_ptr;
};

} // namespace Cutelyst

#endif // ACTIONREST_P_H
