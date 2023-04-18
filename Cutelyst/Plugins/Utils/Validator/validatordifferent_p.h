/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDIFFERENT_P_H
#define CUTELYSTVALIDATORDIFFERENT_P_H

#include "validatordifferent.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorDifferentPrivate : public ValidatorRulePrivate
{
public:
    ValidatorDifferentPrivate(const QString &f, const QString &of, const char *ol, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
        , otherField(of)
        , otherLabel(ol)
    {
    }

    QString otherField;
    const char *otherLabel = nullptr;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDIFFERENT_P_H
