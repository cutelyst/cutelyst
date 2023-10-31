/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORSIZE_P_H
#define CUTELYSTVALIDATORSIZE_P_H

#include "validatorrule_p.h"
#include "validatorsize.h"

namespace Cutelyst {

class ValidatorSizePrivate : public ValidatorRulePrivate
{
public:
    ValidatorSizePrivate(const QString &f,
                         QMetaType::Type t,
                         QVariant s,
                         const ValidatorMessages &m,
                         const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk, "ValidatorSize")
        , type(t)
        , size(std::move(s))
    {
    }

    QMetaType::Type type;
    QVariant size;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORSIZE_P_H
