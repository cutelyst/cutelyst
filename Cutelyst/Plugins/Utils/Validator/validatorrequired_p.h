/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIRED_P_H
#define CUTELYSTVALIDATORREQUIRED_P_H

#include "validatorrequired.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredPrivate(const QString &f, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
    {
    }
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIRED_P_H
