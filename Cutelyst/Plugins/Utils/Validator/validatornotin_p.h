/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
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
    ValidatorNotInPrivate(const QString &f, const QStringList &v, Qt::CaseSensitivity s, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , cs(s)
        , values(v)
    {
    }

    Qt::CaseSensitivity cs;
    QStringList values;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORNOTIN_P_H
