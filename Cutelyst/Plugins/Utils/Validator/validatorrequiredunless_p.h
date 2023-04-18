/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDUNLESS_P_H
#define CUTELYSTVALIDATORREQUIREDUNLESS_P_H

#include "validatorrequiredunless.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredUnlessPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredUnlessPrivate(const QString &f, const QString &of, const QStringList &ov, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
        , otherField(of)
        , otherValues(ov)
    {
    }

    QString otherField;
    QStringList otherValues;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDUNLESS_P_H
