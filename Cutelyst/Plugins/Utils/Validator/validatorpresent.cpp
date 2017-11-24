﻿/*
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

#include "validatorpresent_p.h"

using namespace Cutelyst;

ValidatorPresent::ValidatorPresent(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorPresentPrivate(field, label, customError))
{
}

ValidatorPresent::ValidatorPresent(ValidatorPresentPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorPresent::~ValidatorPresent()
{
}

QString ValidatorPresent::validate() const
{
    QString result;

    if (!parameters().contains(field())) {
        result = validationError();
    }

    return result;
}

QString ValidatorPresent::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Has to be present in input data.");
    } else {
        error = QStringLiteral("The “%1” field was not found in the input data.").arg(label());
    }
    return error;
}
