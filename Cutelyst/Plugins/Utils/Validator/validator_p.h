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
#ifndef CUTELYSTVALIDATOR_P_H
#define CUTELYSTVALIDATOR_P_H

#include "validator.h"
#include "validatorrule.h"
#include <QHash>

namespace Cutelyst {

class ValidatorPrivate
{
public:
    ValidatorPrivate(const QLatin1String &trContext) :
        translationContext(trContext)
    {}

#ifdef Q_COMPILER_INITIALIZER_LISTS
    ValidatorPrivate(std::initializer_list<ValidatorRule*> vals, const QLatin1String &trContext) :
        translationContext(trContext),
        validators(vals)
    {
        if (!validators.empty()) {
            for (ValidatorRule* r : validators) {
                r->setTranslationContext(trContext);
            }
        }
    }
#endif

    ~ValidatorPrivate() {
        if (!validators.empty()) {
            qDeleteAll(validators.begin(), validators.end());
            validators.clear();
        }
    }

    QLatin1String translationContext;
    ParamsMultiMap params;
    std::vector<ValidatorRule*> validators;
};

}

#endif //CUTELYSTVALIDATOR_P_H

