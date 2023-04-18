/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORJSON_P_H
#define CUTELYSTVALIDATORJSON_P_H

#include "validatorjson.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorJsonPrivate : public ValidatorRulePrivate
{
public:
    ValidatorJsonPrivate(const QString &f, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
    {
    }
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORJSON_P_H
