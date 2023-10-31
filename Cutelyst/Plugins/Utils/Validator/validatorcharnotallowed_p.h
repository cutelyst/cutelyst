/*
 * SPDX-FileCopyrightText: (C) 2019-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORCHARNOTALLOWED_P_H
#define CUTELYSTVALIDATORCHARNOTALLOWED_P_H

#include "validatorcharnotallowed.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorCharNotAllowedPrivate : public ValidatorRulePrivate
{
public:
    ValidatorCharNotAllowedPrivate(const QString &f,
                                   QString fcs,
                                   const ValidatorMessages &m,
                                   const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorCharNotAllowed")
        , forbiddenChars(std::move(fcs))
    {
    }

    QString forbiddenChars;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORCHARNOTALLOWED_P_H
