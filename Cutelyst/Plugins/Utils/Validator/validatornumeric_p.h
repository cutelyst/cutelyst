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
#ifndef CUTELYSTVALIDATORNUMERIC_P_H
#define CUTELYSTVALIDATORNUMERIC_P_H

#include "validatornumeric.h"
#include "validatorrule_p.h"

namespace Cutelyst {
    
class ValidatorNumericPrivate : public ValidatorRulePrivate
{
public:
    ValidatorNumericPrivate(const QString &f, const QString &l, const QString &e) :
        ValidatorRulePrivate(f, l, e)
    {}
};
    
}

#endif //CUTELYSTVALIDATORNUMERIC_P_H

