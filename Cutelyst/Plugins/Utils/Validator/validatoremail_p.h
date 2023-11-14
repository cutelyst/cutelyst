/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
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
        : ValidatorRulePrivate(f, m, dvk, "ValidatorEmail")
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
    static constexpr char16_t asciiLF{10};
    static constexpr char16_t asciiUS{31};
    static constexpr char16_t asciiExclamationMark{33};
    static constexpr char16_t asciiTilde{126};
    static constexpr char16_t asciiEnd{127};

    // https://tools.ietf.org/html/rfc5321#section-4.5.3.1.1
    // The maximum total length of a user name or other local-part is 64 octets.
    static constexpr qsizetype maxLocalPartLength{64};
    // https://tools.ietf.org/html/rfc1035#section-2.3.4
    // labels          63 octets or less
    static constexpr qsizetype maxDnsLabelLength{63};
    // There is a restriction in RFC 2821 on the length of an
    // address in MAIL and RCPT commands of 254 characters. Since addresses
    // that do not fit in those fields are not normally useful, the upper
    // limit on address lengths should normally be considered to be 254.
    static constexpr qsizetype maxMailboxLength{254};
    // https://tools.ietf.org/html/rfc5321#section-4.5.3.1.2
    // The maximum total length of a domain name or number is 255 octets.
    static constexpr qsizetype maxDomainLength{255};

    static constexpr std::chrono::milliseconds dnsLookupTimeout{3100};

    static const QRegularExpression ipv4Regex;
    static const QRegularExpression ipv6PartRegex;

    // US-ASCII visible characters not valid for atext
    // (https://tools.ietf.org/html/rfc5322#section-3.2.3)
    static const QString stringSpecials;

    ValidatorEmail::Category threshold = ValidatorEmail::RFC5321;
    ValidatorEmail::Options options    = ValidatorEmail::NoOption;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATOREMAIL_P_H
