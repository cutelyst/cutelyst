/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORREQUIREDWITHALL_P_H
#define CUTELYSTVALIDATORREQUIREDWITHALL_P_H

#include "validatorrequiredwithall.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorRequiredWithAllPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredWithAllPrivate(const QString &f, const QStringList &of, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
        , otherFields(of)
    {
    }

    QStringList otherFields;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORREQUIREDWITHALL_P_H
