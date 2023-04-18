/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORINTEGER_P_H
#define CUTELYSTVALIDATORINTEGER_P_H

#include "validatorinteger.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorIntegerPrivate : public ValidatorRulePrivate
{
public:
    ValidatorIntegerPrivate(const QString &f, QMetaType::Type t, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , type(t)
    {
    }

    QMetaType::Type type = QMetaType::ULongLong;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORINTEGER_P_H
