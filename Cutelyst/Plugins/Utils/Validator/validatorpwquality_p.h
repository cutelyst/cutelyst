/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORPWQUALITY_P_H
#define CUTELYSTVALIDATORPWQUALITY_P_H

#include "validatorpwquality.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorPwQualityPrivate : public ValidatorRulePrivate
{
public:
    ValidatorPwQualityPrivate(const QString &f, int t, const QVariant &opts, const QString &un, const QString &opw, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString())
        , options(opts)
        , userName(un)
        , oldPassword(opw)
        , threshold(t)
    {
    }

    QVariant options;
    QString userName;
    QString oldPassword;
    int threshold = 30;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORPWQUALITY_P_H
