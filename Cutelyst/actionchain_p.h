/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ACTIONCHAIN_P_H
#define ACTIONCHAIN_P_H

#include "action_p.h"
#include "actionchain.h"

namespace Cutelyst {

class ActionChainPrivate : public ActionPrivate
{
public:
    virtual ~ActionChainPrivate() override = default;

    ActionList chain;
    qint8 captures = 0;
};

} // namespace Cutelyst

#endif // ACTIONCHAIN_P_H
