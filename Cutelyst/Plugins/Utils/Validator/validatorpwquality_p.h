/*
 * SPDX-FileCopyrightText: (C) 2018-2023 Matthias Fehring <mf@huessenbergnetz.de>
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
    ValidatorPwQualityPrivate(const QString &f,
                              int t,
                              QVariant opts,
                              QString un,
                              QString opw,
                              const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString(), "ValidatorPwQuality")
        , options(std::move(opts))
        , userName(std::move(un))
        , oldPassword(std::move(opw))
        , threshold(t)
    {
    }

    static constexpr qsizetype errStrBufSize{1024};

    QVariant options;
    QString userName;
    QString oldPassword;
    int threshold{ValidatorPwQuality::defaultThreshold};
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORPWQUALITY_P_H
