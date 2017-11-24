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
#ifndef CUTELYSTVALIDATORBEFORE_P_H
#define CUTELYSTVALIDATORBEFORE_P_H

#include "validatorbefore.h"
#include "validatorrule_p.h"
#include <QVariant>

namespace Cutelyst {

class ValidatorBeforePrivate : public ValidatorRulePrivate
{
public:
    ValidatorBeforePrivate(const QString &f, const QVariant &d, const QString &i, const QString &l, const QString &e) :
        ValidatorRulePrivate(f, l, e),
        date(d),
        inputFormat(i)
    {}

    QVariant date;
    QString inputFormat;
};

}

#endif //CUTELYSTVALIDATORBEFORE_P_H
