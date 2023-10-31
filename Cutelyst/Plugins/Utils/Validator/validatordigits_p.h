/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDIGITS_P_H
#define CUTELYSTVALIDATORDIGITS_P_H

#include "validatordigits.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorDigitsPrivate : public ValidatorRulePrivate
{
public:
    ValidatorDigitsPrivate(const QString &f,
                           QVariant len,
                           const ValidatorMessages &m,
                           const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorDigits")
        , length(std::move(len))
    {
    }

    QVariant length;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDIGITS_P_H
