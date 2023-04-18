/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORFILLED_P_H
#define CUTELYSTVALIDATORFILLED_P_H

#include "validatorfilled.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorFilledPrivate : public ValidatorRulePrivate
{
public:
    ValidatorFilledPrivate(const QString &f, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
    {
    }
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORFILLED_P_H
