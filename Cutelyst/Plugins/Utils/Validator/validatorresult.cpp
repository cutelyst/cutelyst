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

#include "validatorresult_p.h"

using namespace Cutelyst;

ValidatorResult::ValidatorResult() :
    d(new ValidatorResultPrivate)
{
}

ValidatorResult::ValidatorResult(const ValidatorResult &other) :
    d(other.d)
{
}

ValidatorResult::~ValidatorResult()
{
}

bool ValidatorResult::isValid() const
{
    return d->errorFields.isEmpty();
}

void ValidatorResult::addError(const QString &field, const QString &message)
{
    d->errorFields.append(field);
    d->errorStrings.append(message);
}

QStringList ValidatorResult::errorFields() const
{
    return d->errorFields;
}

QStringList ValidatorResult::errorStrings() const
{
    return d->errorStrings;
}
