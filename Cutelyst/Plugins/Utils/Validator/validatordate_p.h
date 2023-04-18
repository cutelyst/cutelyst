/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDATE_P_H
#define CUTELYSTVALIDATORDATE_P_H

#include "validatordate.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorDatePrivate : public ValidatorRulePrivate
{
public:
    ValidatorDatePrivate(const QString &f, const char *i, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , inputFormat(i)
    {
    }

    const char *inputFormat = nullptr;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDATE_P_H
