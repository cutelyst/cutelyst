/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREGEX_P_H
#define CUTELYSTVALIDATORREGEX_P_H

#include "validatorregularexpression.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRegularExpressionPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRegularExpressionPrivate(const QString &f,
                                      QRegularExpression r,
                                      const ValidatorMessages &m,
                                      const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorRegularExpression")
        , regex(std::move(r))
    {
    }

    QRegularExpression regex;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREGEX_P_H
