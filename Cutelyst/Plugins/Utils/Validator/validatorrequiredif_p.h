/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDIF_P_H
#define CUTELYSTVALIDATORREQUIREDIF_P_H

#include "validatorrequiredif.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredIfPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredIfPrivate(const QString &f,
                               QString of,
                               QStringList ov,
                               const Cutelyst::ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString(), "ValidatorRequiredIf")
        , otherField(std::move(of))
        , otherValues(std::move(ov))
    {
    }

    QString otherField;
    QStringList otherValues;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDIF_P_H
