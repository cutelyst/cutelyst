/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORBETWEEN_P_H
#define CUTELYSTVALIDATORBETWEEN_P_H

#include "validatorbetween.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorBetweenPrivate : public ValidatorRulePrivate
{
public:
    ValidatorBetweenPrivate(const QString &f, QMetaType::Type t, const QVariant &mi, const QVariant &ma, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , min(mi)
        , max(ma)
        , type(t)
    {
    }

    QVariant min;
    QVariant max;
    QMetaType::Type type;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORBETWEEN_P_H
