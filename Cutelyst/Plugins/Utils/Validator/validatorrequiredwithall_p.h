/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHALL_P_H
#define CUTELYSTVALIDATORREQUIREDWITHALL_P_H

#include "validatorrequiredwithall.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredWithAllPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredWithAllPrivate(const QString &f, QStringList of, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, {}, "ValidatorRequiredWithAll")
        , otherFields(std::move(of))
    {
    }

    QStringList otherFields;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHALL_P_H
