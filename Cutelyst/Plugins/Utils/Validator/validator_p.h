/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
    ValidatorPrivate() {}

#ifdef Q_COMPILER_INITIALIZER_LISTS
    ValidatorPrivate(std::initializer_list<ValidatorRule*> vals) :
        validators(vals)
    {}

    ValidatorPrivate(std::initializer_list<ValidatorRule*> vals, std::initializer_list<std::pair<QString, QString> > labelDictionary) :
        validators(vals),
        labelDict(labelDictionary)
    {}
#endif

    ~ValidatorPrivate() {
        if (!validators.empty()) {
            qDeleteAll(validators.begin(), validators.end());
            validators.clear();
        }
    }

    ParamsMultiMap params;
    std::vector<ValidatorRule*> validators;
    QHash<QString,QString> labelDict;
};

}

#endif //CUTELYSTVALIDATOR_P_H

