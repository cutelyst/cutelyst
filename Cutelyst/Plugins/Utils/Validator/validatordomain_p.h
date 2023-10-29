/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDOMAIN_P_H
#define CUTELYSTVALIDATORDOMAIN_P_H

#include "validatordomain.h"
#include "validatorrule_p.h"

#include <chrono>

namespace Cutelyst {

class ValidatorDomainPrivate : public ValidatorRulePrivate
{
public:
    ValidatorDomainPrivate(const QString &f,
                           bool cd,
                           const ValidatorMessages &m,
                           const QString &dvk)
        : ValidatorRulePrivate(f, m, dvk)
        , checkDNS(cd)
    {
    }

    static constexpr qsizetype maxDnsNameWithLastDot{253};
    static constexpr qsizetype maxDnsLabelLength{63};
    static constexpr std::chrono::milliseconds dnsLookupTimeout{3100};

    bool checkDNS = false;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDOMAIN_P_H
