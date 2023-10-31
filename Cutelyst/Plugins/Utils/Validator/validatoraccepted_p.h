/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORACCEPTED_P_H
#define CUTELYSTVALIDATORACCEPTED_P_H

#include "validatoraccepted.h"
#include "validatorrule_p.h"

namespace Cutelyst {

class ValidatorAcceptedPrivate : public ValidatorRulePrivate
{
public:
    ValidatorAcceptedPrivate(const QString &f, const ValidatorMessages &m)
        : ValidatorRulePrivate(f, m, QString(), "ValidatorAccepted")
    {
    }

    static const QStringList trueVals;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORACCEPTED_P_H
