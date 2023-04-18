/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORFILESIZE_P_H
#define CUTELYSTVALIDATORFILESIZE_P_H

#include "validatorfilesize.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorFileSizePrivate : public ValidatorRulePrivate
{
public:
    ValidatorFileSizePrivate(const QString &f, ValidatorFileSize::Option o, const QVariant &mi, const QVariant &ma, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , min(mi)
        , max(ma)
        , option(o)
    {
    }

    QVariant min;
    QVariant max;
    ValidatorFileSize::Option option = ValidatorFileSize::NoOption;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORFILESIZE_P_H
