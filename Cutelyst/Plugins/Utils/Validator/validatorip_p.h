/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORIP_P_H
#define CUTELYSTVALIDATORIP_P_H

#include "validatorip.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorIpPrivate : public ValidatorRulePrivate
{
public:
    ValidatorIpPrivate(const QString &f, ValidatorIp::Constraints c, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , constraints(c)
    {
    }

    ValidatorIp::Constraints constraints;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORIP_P_H
