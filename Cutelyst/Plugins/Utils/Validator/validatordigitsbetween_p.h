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
#ifndef CUTELYSTVALIDATORDIGITSBETWEEN_P_H
#define CUTELYSTVALIDATORDIGITSBETWEEN_P_H

#include "validatordigitsbetween.h"
#include "validatorrule_p.h"

namespace Cutelyst {
    
class ValidatorDigitsBetweenPrivate : public ValidatorRulePrivate
{
public:
    ValidatorDigitsBetweenPrivate(const QString &f, const QVariant &mi, const QVariant &ma, const ValidatorMessages &m, const QString &dvk) :
        ValidatorRulePrivate(f, m, dvk),
        min(mi),
        max(ma)
    {}

    QVariant min;
    QVariant max;
};
    
}

#endif //CUTELYSTVALIDATORDIGITSBETWEEN_P_H

