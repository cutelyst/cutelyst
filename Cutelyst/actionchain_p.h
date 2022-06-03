/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ACTIONCHAIN_P_H
#define ACTIONCHAIN_P_H

#include "actionchain.h"
#include "action_p.h"

namespace Cutelyst {

class ActionChainPrivate : public ActionPrivate
{
public:
    virtual ~ActionChainPrivate() override = default;

    ActionList chain;
    qint8 captures = 0;
};

}

#endif // ACTIONCHAIN_P_H
