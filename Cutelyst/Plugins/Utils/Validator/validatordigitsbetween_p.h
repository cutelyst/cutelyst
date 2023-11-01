/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDIGITSBETWEEN_P_H
#define CUTELYSTVALIDATORDIGITSBETWEEN_P_H

#include "validatordigitsbetween.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorDigitsBetweenPrivate : public ValidatorRulePrivate
{
public:
    ValidatorDigitsBetweenPrivate(const QString &f,
                                  QVariant mi,
                                  QVariant ma,
                                  const ValidatorMessages &m,
                                  const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , min(std::move(mi))
        , max(std::move(ma))
    {
    }

    QVariant min;
    QVariant max;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDIGITSBETWEEN_P_H
