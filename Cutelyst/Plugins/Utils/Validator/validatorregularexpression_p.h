/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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
#ifndef CUTELYSTVALIDATORREGEX_P_H
#define CUTELYSTVALIDATORREGEX_P_H

#include "validatorregularexpression.h"
#include "validatorrule_p.h"

namespace Cutelyst {
    
class ValidatorRegularExpressionPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRegularExpressionPrivate(const QString &f, const QRegularExpression &r, const QString &l, const QString &e) :
        ValidatorRulePrivate(f, l, e),
        regex(r)
    {}

    QRegularExpression regex;
};
    
}

#endif //CUTELYSTVALIDATORREGEX_P_H

