/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATOR_P_H
#define CUTELYSTVALIDATOR_P_H

#include "validator.h"
#include "validatorrule.h"

#include <QHash>

namespace Cutelyst {

class ValidatorPrivate
{
public:
    ValidatorPrivate(QLatin1String trContext)
        : translationContext(trContext)
    {
    }

#ifdef Q_COMPILER_INITIALIZER_LISTS
    ValidatorPrivate(std::initializer_list<ValidatorRule *> vals, QLatin1String trContext)
        : translationContext(trContext)
        , validators(vals)
    {
        if (!validators.empty()) {
            for (ValidatorRule *r : validators) {
                r->setTranslationContext(trContext);
            }
        }
    }
#endif

    ~ValidatorPrivate()
    {
        if (!validators.empty()) {
            qDeleteAll(validators.begin(), validators.end());
            validators.clear();
        }
    }

    QLatin1String translationContext;
    ParamsMultiMap params;
    std::vector<ValidatorRule *> validators;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATOR_P_H
