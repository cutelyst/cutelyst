/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
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
    ValidatorJsonPrivate(const QString &f,
                         ValidatorJson::ExpectedType expType,
                         const ValidatorMessages &m,
                         const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorJson")
        , expectedType(expType)
    {
    }

    ValidatorJson::ExpectedType expectedType{ValidatorJson::ExpectedType::All};
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORJSON_P_H
