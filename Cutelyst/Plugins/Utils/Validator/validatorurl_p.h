/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORURL_P_H
#define CUTELYSTVALIDATORURL_P_H

#include "validatorurl.h"
#include "validatorrule_p.h"

namespace Cutelyst {
    
class ValidatorUrlPrivate : public ValidatorRulePrivate
{
public:
    ValidatorUrlPrivate(const QString &f, ValidatorUrl::Constraints c, const QStringList &s, const Cutelyst::ValidatorMessages &m, const QString &dvk) :
        ValidatorRulePrivate(f, m, dvk),
        constraints(c),
        schemes(s)
    {}

    ValidatorUrl::Constraints constraints;
    QStringList schemes;
};
    
}

#endif //CUTELYSTVALIDATORURL_P_H

