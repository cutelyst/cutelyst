/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORTIME_P_H
#define CUTELYSTVALIDATORTIME_P_H

#include "validatorrule_p.h"
#include "validatortime.h"

namespace Cutelyst {

class ValidatorTimePrivate : public ValidatorRulePrivate
{
public:
    ValidatorTimePrivate(const QString &f,
                         const char *fo,
                         const ValidatorMessages &m,
                         const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorTime")
        , format(fo)
    {
    }

    const char *format = nullptr;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORTIME_P_H
