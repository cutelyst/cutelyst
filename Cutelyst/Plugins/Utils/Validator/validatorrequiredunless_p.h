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
#ifndef CUTELYSTVALIDATORREQUIREDUNLESS_P_H
#define CUTELYSTVALIDATORREQUIREDUNLESS_P_H

#include "validatorrequiredunless.h"
#include "validatorrule_p.h"

namespace Cutelyst {
    
class ValidatorRequiredUnlessPrivate : public ValidatorRulePrivate
{
public:
    ValidatorRequiredUnlessPrivate(const QString &f, const QString &of, const QStringList &ov, const ValidatorMessages &m) :
        ValidatorRulePrivate(f, m, QString()),
        otherField(of),
        otherValues(ov)
    {}

    QString otherField;
    QStringList otherValues;
};
    
}

#endif //CUTELYSTVALIDATORREQUIREDUNLESS_P_H

