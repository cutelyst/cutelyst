/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORBOOLEAN_P_H
#define CUTELYSTVALIDATORBOOLEAN_P_H

#include "validatorboolean.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorBooleanPrivate : public ValidatorRulePrivate
{
public:
    ValidatorBooleanPrivate(const QString &f, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorBoolean")
    {
    }

    static const QStringList trueVals;
    static const QStringList falseVals;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORBOOLEAN_P_H
