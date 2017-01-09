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

#include "validatordigits_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorDigits::ValidatorDigits(const QString &field, int length, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorDigitsPrivate(field, length, label, customError), parent)
{

}


ValidatorDigits::ValidatorDigits(ValidatorDigitsPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorDigits::~ValidatorDigits()
{

}



bool ValidatorDigits::validate()
{
    Q_D(ValidatorDigits);

    if (value().isEmpty()) {
        setValid(true);
        return true;
    }

    if (value().contains(QRegularExpression(QStringLiteral("^[0-9]+$")))) {
        if (d->length > 0) {
            if (value().length() == d->length) {
                setValid(true);
                return true;
            } else {
                return false;
            }
        } else {
            setValid(true);
            return true;
        }
    } else {
        return false;
    }
}



QString ValidatorDigits::genericErrorMessage() const
{
    Q_D(const ValidatorDigits);    if (d->length > 0) {
        return tr("The “%1” field must only contain exactly %2 digits.").arg(genericFieldName(), QString::number(d->length));
    } else {
        return tr("The “%1” field must only contain digits.").arg(genericFieldName());
    }
}


void ValidatorDigits::setLength(int length)
{
    Q_D(ValidatorDigits);
    d->length = length;
}
