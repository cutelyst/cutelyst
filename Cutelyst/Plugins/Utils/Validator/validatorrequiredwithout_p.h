/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUT_P_H
#define CUTELYSTVALIDATORREQUIREDWITHOUT_P_H

#include "validatorrequiredwithout.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredWithoutPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredWithoutPrivate(const QString &f, QStringList of, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, {}, "ValidatorRequiredWithout")
        , otherFields(std::move(of))
    {
    }

    QStringList otherFields;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHOUT_P_H
