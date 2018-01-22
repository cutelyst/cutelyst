/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
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
    ValidatorEmailPrivate(const QString &f, ValidatorEmail::Category thresh, bool dns, const ValidatorMessages &m, const QString &dvk) :
        ValidatorRulePrivate(f, m, dvk),
        threshold(thresh),
        checkDns(dns)
    {}

    enum EmailPart {
        ComponentLocalpart  = 0,
        ComponentDomain     = 1,
        ComponentLiteral    = 2,
        ContextComment      = 3,
        ContextFWS          = 4,
        ContextQuotedString = 5,
        ContextQuotedPair   = 6
    };

    static bool checkEmail(const QString &email, bool checkDNS = false, ValidatorEmail::Category threshold = ValidatorEmail::RFC5321, ValidatorEmailDiagnoseStruct *diagnoseStruct = nullptr);

    ValidatorEmail::Category threshold = ValidatorEmail::RFC5321;
    bool checkDns = false;
};
    
}

#endif //CUTELYSTVALIDATOREMAIL_P_H

