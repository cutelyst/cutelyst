/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORAFTER_P_H
#define CUTELYSTVALIDATORAFTER_P_H

#include "validatorafter.h"
#include "validatorrule_p.h"

#include <QVariant>

namespace Cutelyst {

class ValidatorAfterPrivate : public ValidatorRulePrivate
{
public:
    ValidatorAfterPrivate(const QString &f, const QVariant &comp, const QString &tz, const char *i, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , comparison(comp)
        , timeZone(tz)
        , inputFormat(i)
    {
    }

    QVariant comparison;
    QString timeZone;
    const char *inputFormat = nullptr;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORAFTER_P_H
