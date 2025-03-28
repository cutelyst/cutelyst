/*
 * SPDX-FileCopyrightText: (C) 2018-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORREQUIREDUNLESSSTASH_P_H
#define CUTELYSTVALIDATORREQUIREDUNLESSSTASH_P_H

#include "validatorrequiredunlessstash.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredUnlessStashPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredUnlessStashPrivate(const QString &f,
                                        QString sk,
                                        QVariant sv,
                                        const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, {}, "ValidatorRequiredUnlessStash")
        , stashKey(std::move(sk))
        , stashValues(std::move(sv))
    {
    }

    QString stashKey;
    QVariant stashValues;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDUNLESSSTASH_P_H
