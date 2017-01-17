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

#include "validatormax_p.h"

using namespace Cutelyst;

ValidatorMax::ValidatorMax(const QString &field, QMetaType::Type type, double max, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorMaxPrivate(field, type, max, label, customError))
{
}

ValidatorMax::ValidatorMax(ValidatorMaxPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorMax::~ValidatorMax()
{
}

QString ValidatorMax::validate() const
{
    QString result;

    const QString v = value();

    if (!v.isEmpty()) {
        Q_D(const ValidatorMax);

        if (d->type == QMetaType::Int) {
            qlonglong val = v.toLongLong();
            qlonglong max = (qlonglong)d->max;
            if (val > max) {
                result = validationError();
            }
        } else if (d->type == QMetaType::UInt) {
            qulonglong val = v.toULongLong();
            qulonglong max = (qulonglong)d->max;
            if (val > max) {
                result = validationError();
            }
        } else if (d->type == QMetaType::Float) {
            double val = v.toDouble();
            if (val > d->max) {
                result = validationError();
            }
        } else if (d->type == QMetaType::QString) {
            int val = v.length();
            int max = (int)d->max;
            if (val > max) {
                result = validationError();
            }
        } else {
            result = validationDataError();
        }
    }

    return result;
}

QString ValidatorMax::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorMax);

    if (d->type == QMetaType::Int || d->type == QMetaType::UInt) {
        error = QStringLiteral("The value of the “%1” field has to be lower than or equal to %2.").arg(fieldLabel(),QString::number(d->max, 'f', 0));
    } else if (d->type == QMetaType::Float) {
        error =  QStringLiteral("The value of the “%1” field has to be lower than or equal to %2.").arg(fieldLabel(), QString::number(d->max));
    } else if (d->type == QMetaType::QString) {
        error =  QStringLiteral("The length of the “%1” field has to be lower than or equal to %2.").arg(fieldLabel(), QString::number(d->max, 'f', 0));
    } else {
        error =  validationDataError();
    }

    return error;
}

void ValidatorMax::setType(QMetaType::Type type)
{
    Q_D(ValidatorMax);
    d->type = type;
}

void ValidatorMax::setMax(double max)
{
    Q_D(ValidatorMax);
    d->max = max;
}
