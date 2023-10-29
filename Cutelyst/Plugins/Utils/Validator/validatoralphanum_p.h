/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORALPHANUM_P_H
#define CUTELYSTVALIDATORALPHANUM_P_H

#include "validatoralphanum.h"
#include "validatorrule_p.h"

#include <QRegularExpression>

namespace Cutelyst {

class ValidatorAlphaNumPrivate : public ValidatorRulePrivate
{
public:
    ValidatorAlphaNumPrivate(const QString &f,
                             bool ao,
                             const ValidatorMessages &m,
                             const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , asciiOnly(ao)
    {
    }

    static const QRegularExpression regex;

    bool asciiOnly = false;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORALPHANUM_P_H
