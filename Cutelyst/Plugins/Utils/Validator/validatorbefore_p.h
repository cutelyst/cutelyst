/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORBEFORE_P_H
#define CUTELYSTVALIDATORBEFORE_P_H

#include "validatorbefore.h"
#include "validatorrule_p.h"

#include <QVariant>

namespace Cutelyst {

class ValidatorBeforePrivate : public ValidatorRulePrivate
{
public:
    ValidatorBeforePrivate(const QString &f, const QVariant &comp, const QString &tz, const char *i, const ValidatorMessages &m, const QString &dvk)
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

#endif // CUTELYSTVALIDATORBEFORE_P_H
