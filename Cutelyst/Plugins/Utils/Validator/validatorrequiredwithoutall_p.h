/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHOUTALL_P_H
#define CUTELYSTVALIDATORREQUIREDWITHOUTALL_P_H

#include "validatorrequiredwithoutall.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredWithoutAllPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredWithoutAllPrivate(const QString &f, QStringList o, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString(), "ValidatorRequiredWithoutAll")
        , otherFields(std::move(o))
    {
    }

    QStringList otherFields;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHOUTALL_P_H
