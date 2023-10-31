/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORSAME_P_H
#define CUTELYSTVALIDATORSAME_P_H

#include "validatorrule_p.h"
#include "validatorsame.h"

namespace Cutelyst {

class ValidatorSamePrivate : public ValidatorRulePrivate
{
public:
    ValidatorSamePrivate(const QString &f,
                         QString o,
                         const char *ol,
                         const ValidatorMessages &m,
                         const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorSame")
        , otherLabel(ol)
        , otherField(std::move(o))
    {
    }

    const char *otherLabel = nullptr;
    QString otherField;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORSAME_P_H
