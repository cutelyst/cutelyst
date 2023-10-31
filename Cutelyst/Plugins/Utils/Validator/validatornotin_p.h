/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORNOTIN_P_H
#define CUTELYSTVALIDATORNOTIN_P_H

#include "validatornotin.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorNotInPrivate : public ValidatorRulePrivate
{
public:
    ValidatorNotInPrivate(const QString &f,
                          QVariant v,
                          Qt::CaseSensitivity s,
                          const ValidatorMessages &m,
                          const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorNotIn")
        , cs(s)
        , values(std::move(v))
    {
    }

    Qt::CaseSensitivity cs;
    QVariant values;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORNOTIN_P_H
