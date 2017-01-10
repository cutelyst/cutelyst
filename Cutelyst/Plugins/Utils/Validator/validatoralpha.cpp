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

#include "validatoralpha_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlpha::ValidatorAlpha(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorAlphaPrivate(field, label, customError))
{

}

ValidatorAlpha::ValidatorAlpha(ValidatorAlphaPrivate &dd) :
    ValidatorRule(dd)
{

}

ValidatorAlpha::~ValidatorAlpha()
{

}

bool ValidatorAlpha::validate()
{
    if (value().isEmpty()) {
        setError(ValidatorRule::NoError);
        return true;
    }

    if (value().contains(QRegularExpression(QStringLiteral("^[\\pL\\pM]+$")))) {
        setError(ValidatorRule::NoError);
        return true;
    } else {
        return false;
    }
}

QString ValidatorAlpha::genericErrorMessage() const
{
    return QStringLiteral("The text in the “%1” field must be entirely alphabetic characters.").arg(genericFieldName());
}
