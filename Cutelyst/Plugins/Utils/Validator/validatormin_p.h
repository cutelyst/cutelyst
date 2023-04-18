/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORMIN_P_H
#define CUTELYSTVALIDATORMIN_P_H

#include "validatormin.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorMinPrivate : public ValidatorRulePrivate
{
public:
    ValidatorMinPrivate(const QString &f, QMetaType::Type t, const QVariant &m, const ValidatorMessages &msgs, const QString &dvk)
        : ValidatorRulePrivate(f, msgs, dvk)
        , type(t)
        , min(m)
    {
    }

    QMetaType::Type type;
    QVariant min;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORMIN_P_H
