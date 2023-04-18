/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORNUMERIC_P_H
#define CUTELYSTVALIDATORNUMERIC_P_H

#include "validatornumeric.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorNumericPrivate : public ValidatorRulePrivate
{
public:
    ValidatorNumericPrivate(const QString &f, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
    {
    }
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORNUMERIC_P_H
