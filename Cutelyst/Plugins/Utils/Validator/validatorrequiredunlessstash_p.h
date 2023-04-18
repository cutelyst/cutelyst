/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
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
    ValidatorRequiredUnlessStashPrivate(const QString &f, const QString &sk, const QVariantList &sv, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
        , stashKey(sk)
        , stashValues(sv)
    {
    }

    QString stashKey;
    QVariantList stashValues;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDUNLESSSTASH_P_H
