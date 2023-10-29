/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATOREMAIL_P_H
#define CUTELYSTVALIDATOREMAIL_P_H

#include "validatoremail.h"
#include "validatorrule_p.h"

#include <QRegularExpression>

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
    ValidatorEmailPrivate(const QString &f,
                          ValidatorEmail::Category thresh,
                          ValidatorEmail::Options opts,
                          const ValidatorMessages &m,
                          const QString &dvk)
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

    static bool checkEmail(const QString &address,
                           ValidatorEmail::Options options              = ValidatorEmail::NoOption,
                           ValidatorEmail::Category threshold           = ValidatorEmail::RFC5321,
                           ValidatorEmailDiagnoseStruct *diagnoseStruct = nullptr);

    // last integer code number use by ASCII
    static constexpr char16_t asciiTab{9};
    static constexpr char16_t asciiLF{10};
    static constexpr char16_t asciiUS{31};
    static constexpr char16_t asciiSpace{32};
    static constexpr char16_t asciiExclamationMark{33};
    static constexpr char16_t asciiTilde{126};
    static constexpr char16_t asciiEnd{127};

    // https://tools.ietf.org/html/rfc5321#section-4.5.3.1.1
    // The maximum total length of a user name or other local-part is 64 octets.
    static constexpr qsizetype maxLocalPartLength{64};
    static constexpr qsizetype maxDnsLabelLength{63};

    static constexpr std::chrono::milliseconds dnsLookupTimeout{3100};

    static const QRegularExpression ipv4Regex;
    static const QRegularExpression ipv6PartRegex;

    ValidatorEmail::Category threshold = ValidatorEmail::RFC5321;
    ValidatorEmail::Options options    = ValidatorEmail::NoOption;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATOREMAIL_P_H
