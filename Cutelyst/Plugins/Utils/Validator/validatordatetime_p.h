/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDATETIME_P_H
#define CUTELYSTVALIDATORDATETIME_P_H

#include "validatordatetime.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorDateTimePrivate : public ValidatorRulePrivate
{
public:
    ValidatorDateTimePrivate(const QString &f, const QString &tz, const char *inf, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , timeZone(tz)
        , inputFormat(inf)
    {
    }

    QString timeZone;
    const char *inputFormat = nullptr;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDATETIME_P_H
