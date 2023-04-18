/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
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
    ValidatorRequiredIfPrivate(const QString &f, const QString &of, const QStringList &ov, const Cutelyst::ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
        , otherField(of)
        , otherValues(ov)
    {
    }

    QString otherField;
    QStringList otherValues;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDIF_P_H
