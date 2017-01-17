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

QString ValidatorDigitsBetween::validate() const
{
    QString result;

    Q_D(const ValidatorDigitsBetween);

    if (!value().isEmpty()) {
        if (value().contains(QRegularExpression(QStringLiteral("^[0-9]+$")))) {

            if ((d->min > 0) && (d->max > d->min)) {
                if ((value().length() < d->min) || (value().length() > d->max)) {
                    result = validationError();
                }
            }

        } else {
            result = validationError();
        }
    }

    return result;
}

QString ValidatorDigitsBetween::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorDigitsBetween);
    if (d->min < 1 || d->max < 1) {
        error = QStringLiteral("The “%1” field must only contain digits.").arg(fieldLabel());
    } else {
        error = QStringLiteral("The “%1” field must only contain digits with a length between %2 and %3.").arg(fieldLabel(), QString::number(d->min), QString::number(d->max));
    }

    return error;
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
