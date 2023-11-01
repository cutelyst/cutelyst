/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORURL_P_H
#define CUTELYSTVALIDATORURL_P_H

#include "validatorrule_p.h"
#include "validatorurl.h"

namespace Cutelyst {

class ValidatorUrlPrivate : public ValidatorRulePrivate
{
public:
    ValidatorUrlPrivate(const QString &f,
                        ValidatorUrl::Constraints c,
                        QStringList s,
                        const Cutelyst::ValidatorMessages &m,
                        const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorUrl")
        , constraints(c)
        , schemes(std::move(s))
    {
    }

    ValidatorUrl::Constraints constraints;
    QStringList schemes;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORURL_P_H
