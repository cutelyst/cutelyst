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

#include "validatorbetween_p.h"

using namespace Cutelyst;

ValidatorBetween::ValidatorBetween(const QString &field, QMetaType::Type type, double min, double max, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorBetweenPrivate(field, type, min, max, label, customError))
{
}


ValidatorBetween::ValidatorBetween(ValidatorBetweenPrivate &dd) :
    ValidatorRule(dd)
{
}


ValidatorBetween::~ValidatorBetween()
{
}



QString ValidatorBetween::validate() const
{
    QString result;

    const QString v = value();

    if (!v.isEmpty()) {
        Q_D(const ValidatorBetween);

        if (d->type == QMetaType::Int) {
            qlonglong val = v.toLongLong();
            qlonglong min = (qlonglong)d->min;
            qlonglong max = (qlonglong)d->max;
            if ((val < min) || (val > max)) {
                result = validationError();
            }
        } else if (d->type == QMetaType::UInt) {
            qulonglong val = v.toULongLong();
            qulonglong min = (qulonglong)d->min;
            qulonglong max = (qulonglong)d->max;
            if ((val < min) || (val > max)) {
                result = validationError();
            }
        } else if (d->type == QMetaType::Float) {
            double val = v.toDouble();
            if ((val < d->min) || (val > d->max)) {
                result = validationError();
            }
        } else if (d->type == QMetaType::QString) {
            int val = v.length();
            int min = (int)d->min;
            int max = (int)d->max;
            if ((val < min) || (val > max)) {
                result = validationError();
            }
        } else {
            result = validationDataError();
        }
    }

    return result;
}

QString ValidatorBetween::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorBetween);

    QString min, max;
    if (d->type == QMetaType::Int || d->type == QMetaType::UInt || d->type == QMetaType::QString) {
        min = QString::number(d->min, 'f', 0);
        max = QString::number(d->max, 'f', 0);
    } else {
        min = QString::number(d->min);
        max = QString::number(d->max);
    }

    if (label().isEmpty()) {

        switch (d->type) {
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Float:
            error = QStringLiteral("Value has to be between %1 and %2.").arg(min, max);
            break;
        case QMetaType::QString:
            error = QStringLiteral("Length has to be between %1 and %2.").arg(min, max);
        default:
            error = validationDataError();
            break;
        }

    } else {

        switch (d->type) {
        case QMetaType::Int:
        case QMetaType::UInt:
        case QMetaType::Float:
            error = QStringLiteral("The value of the “%1” field has to be between %2 and %3.").arg(label(), min, max);
            break;
        case QMetaType::QString:
            error = QStringLiteral("The length of the “%1” field has to be between %2 and %3.").arg(label(), min, max);
        default:
            error = validationDataError();
            break;
        }
    }

    return error;
}

void ValidatorBetween::setType(QMetaType::Type type)
{
    Q_D(ValidatorBetween);
    d->type = type;
}

void ValidatorBetween::setMin(double min)
{
    Q_D(ValidatorBetween);
    d->min = min;
}

void ValidatorBetween::setMax(double max)
{
    Q_D(ValidatorBetween);
    d->max = max;
}
