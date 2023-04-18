/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORMAX_P_H
#define CUTELYSTVALIDATORMAX_P_H

#include "validatormax.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorMaxPrivate : public ValidatorRulePrivate
{
public:
    ValidatorMaxPrivate(const QString &f, QMetaType::Type t, const QVariant &m, const ValidatorMessages &msgs, const QString &dvk)
        : ValidatorRulePrivate(f, msgs, dvk)
        , type(t)
        , max(m)
    {
    }

    QMetaType::Type type;
    QVariant max;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORMAX_P_H
