/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORPRESENT_P_H
#define CUTELYSTVALIDATORPRESENT_P_H

#include "validatorpresent.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorPresentPrivate : public ValidatorRulePrivate
{
public:
    ValidatorPresentPrivate(const QString &f, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
    {
    }
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORPRESENT_P_H
