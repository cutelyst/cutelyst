/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
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
    Q_DISABLE_COPY(ValidatorPrivate)
public:
    ValidatorPrivate(const char *trContext)
        : translationContext(trContext)
    {
    }

    ValidatorPrivate(std::initializer_list<ValidatorRule *> vals, const char *trContext)
        : translationContext(trContext)
        , validators(vals)
    {
        if (!validators.empty()) {
            for (ValidatorRule *r : validators) {
                r->setTranslationContext(trContext);
            }
        }
    }

    ~ValidatorPrivate()
    {
        qDeleteAll(validators);
        validators.clear();
    }

    const char *translationContext{nullptr};
    ParamsMultiMap params;
    std::vector<ValidatorRule *> validators;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATOR_P_H
