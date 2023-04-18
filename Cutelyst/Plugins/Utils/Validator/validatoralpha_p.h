/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORALPHA_P_H
#define CUTELYSTVALIDATORALPHA_P_H

#include "validatoralpha.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorAlphaPrivate : public ValidatorRulePrivate
{
public:
    ValidatorAlphaPrivate(const QString &f, bool ao, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , asciiOnly(ao)
    {
    }

    bool asciiOnly = false;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORALPHA_P_H
