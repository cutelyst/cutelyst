/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATOREMAIL_P_H
#define CUTELYSTVALIDATOREMAIL_P_H

#include "validatoremail.h"
#include "validatorrule_p.h"

namespace Cutelyst {

struct ValidatorEmailDiagnoseStruct {
    ValidatorEmail::Diagnose finalStatus;
    QList<ValidatorEmail::Diagnose> returnStatus;
    QString localpart;
    QString domain;
    QString literal;
};

class ValidatorEmailPrivate : public ValidatorRulePrivate
{
public:
    ValidatorEmailPrivate(const QString &f, ValidatorEmail::Category thresh, ValidatorEmail::Options opts, const ValidatorMessages &m, const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , threshold(thresh)
        , options(opts)
    {
    }

    enum EmailPart {
        ComponentLocalpart  = 0,
        ComponentDomain     = 1,
        ComponentLiteral    = 2,
        ContextComment      = 3,
        ContextFWS          = 4,
        ContextQuotedString = 5,
        ContextQuotedPair   = 6
    };

    static bool checkEmail(const QString &address, ValidatorEmail::Options options = ValidatorEmail::NoOption, ValidatorEmail::Category threshold = ValidatorEmail::RFC5321, ValidatorEmailDiagnoseStruct *diagnoseStruct = nullptr);

    ValidatorEmail::Category threshold = ValidatorEmail::RFC5321;
    ValidatorEmail::Options options    = ValidatorEmail::NoOption;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATOREMAIL_P_H
