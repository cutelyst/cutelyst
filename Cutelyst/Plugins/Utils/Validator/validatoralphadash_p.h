/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORALPHADASH_P_H
#define CUTELYSTVALIDATORALPHADASH_P_H

#include "validatoralphadash.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorAlphaDashPrivate : public ValidatorRulePrivate
{
public:
    ValidatorAlphaDashPrivate(const QString &f, bool ao, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , asciiOnly(ao)
    {
    }

    bool asciiOnly = false;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORALPHADASH_P_H
