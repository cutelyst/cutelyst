/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORSAME_P_H
#define CUTELYSTVALIDATORSAME_P_H

#include "validatorsame.h"
#include "validatorrule_p.h"

namespace Cutelyst {
    
class ValidatorSamePrivate : public ValidatorRulePrivate
{
public:
    ValidatorSamePrivate(const QString &f, const QString &o, const char *ol, const ValidatorMessages &m, const QString &dvk) :
        ValidatorRulePrivate(f, m, dvk),
        otherLabel(ol),
        otherField(o)
    {}

    const char * otherLabel = nullptr;
    QString otherField;
};
    
}

#endif //CUTELYSTVALIDATORSAME_P_H

