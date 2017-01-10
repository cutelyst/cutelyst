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

#include "validatordigitsbetween_p.h"

#include <QRegularExpression>

using namespace Cutelyst;

ValidatorDigitsBetween::ValidatorDigitsBetween(const QString &field, int min, int max, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorDigitsBetweenPrivate(field, min, max, label, customError))
{
}

ValidatorDigitsBetween::ValidatorDigitsBetween(ValidatorDigitsBetweenPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorDigitsBetween::~ValidatorDigitsBetween()
{
}

bool ValidatorDigitsBetween::validate()
{
    Q_D(ValidatorDigitsBetween);

    if (value().isEmpty()) {
        setValid(true);
        return true;
    }

    if (value().contains(QRegularExpression(QStringLiteral("^[0-9]+$")))) {

        if (d->min < 1 || d->max < 1) {
            setValid(true);
            return true;
        } else {

            bool res = ((value().length() >= d->min) && (value().length() <= d->max));
            setValid(res);
            return res;
        }

    } else {
        return false;
    }
}

QString ValidatorDigitsBetween::genericErrorMessage() const
{
    Q_D(const ValidatorDigitsBetween);
    if (d->min < 1 || d->max < 1) {
        return QStringLiteral("The “%1” field must only contain digits.").arg(genericFieldName());
    } else {
        return QStringLiteral("The “%1” field must only contain digits with a length between %2 and %3.").arg(genericFieldName(), QString::number(d->min), QString::number(d->max));
    }
}

void ValidatorDigitsBetween::setMin(int min)
{
    Q_D(ValidatorDigitsBetween);
    d->min = min;
}

void ValidatorDigitsBetween::setMax(int max)
{
    Q_D(ValidatorDigitsBetween);
    d->max = max;
}
