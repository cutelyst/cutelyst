/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITH_P_H
#define CUTELYSTVALIDATORREQUIREDWITH_P_H

#include "validatorrequiredwith.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredWithPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredWithPrivate(const QString &f, QStringList o, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, {}, "ValidatorRequiredWith")
        , otherFields(std::move(o))
    {
    }

    QStringList otherFields;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITH_P_H
