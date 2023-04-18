/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORIN_P_H
#define CUTELYSTVALIDATORIN_P_H

#include "validatorin.h"
#include "validatorrule_p.h"

#include <QStringList>

namespace Cutelyst {

class ValidatorInPrivate : public ValidatorRulePrivate
{
public:
    ValidatorInPrivate(const QString &f, const QVariant &vs, Qt::CaseSensitivity c, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , cs(c)
        , values(vs)
    {
    }

    Qt::CaseSensitivity cs = Qt::CaseSensitive;
    QVariant values;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORIN_P_H
