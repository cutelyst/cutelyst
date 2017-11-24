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

#include "validatorsize_p.h"

using namespace Cutelyst;

ValidatorSize::ValidatorSize(const QString &field, QMetaType::Type type, double size, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorSizePrivate(field, type, size, label, customError))
{
}

ValidatorSize::ValidatorSize(ValidatorSizePrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorSize::~ValidatorSize()
{
}

QString ValidatorSize::validate() const
{
    QString result;

    const QString v = value();

    if (!v.isEmpty()) {

        Q_D(const ValidatorSize);

        if (d->type == QMetaType::Int) {
            qlonglong val = v.toLongLong();
            qlonglong size = (qlonglong)d->size;
            if (val != size) {
                result = validationError();
            }
        } else if (d->type == QMetaType::UInt) {
            qulonglong val = v.toULongLong();
            qulonglong size = (qulonglong)d->size;
            if (val != size) {
                result = validationError();
            }
        } else if (d->type == QMetaType::Float) {
            double val = v.toDouble();
            if (val != d->size) {
                result = validationError();
            }
        } else if (d->type == QMetaType::QString) {
            int val = v.length();
            int size = (int)d->size;
            if (val != size) {
                result = validationError();
            }
        } else {
            result = validationDataError();
        }
    }

    return result;
}

QString ValidatorSize::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorSize);

    QString size;
    if (d->type == QMetaType::Int || d->type == QMetaType::UInt || d->type == QMetaType::QString) {
        size = QString::number(d->size, 'f', 0);
    } else {
        size = QString::number(d->size);
    }

    if (label().isEmpty()) {
        if (d->type == QMetaType::Int || d->type == QMetaType::UInt || d->type == QMetaType::Float) {
            error = QStringLiteral("Value has to be equal to %1.").arg(size);
        } else if (d->type == QMetaType::QString) {
            error = QStringLiteral("Length has to be equal to %1.").arg(size);
        } else {
            error = validationDataError();
        }
    } else {
        if (d->type == QMetaType::Int || d->type == QMetaType::UInt || d->type == QMetaType::Float) {
            error = QStringLiteral("The value of the “%1” field has to be equal to %2.").arg(label(), size);
        } else if (d->type == QMetaType::QString) {
            error = QStringLiteral("The length of the “%1” field has to be equal to %2.").arg(label(), size);
        } else {
            error = validationDataError();
        }
    }

    return error;
}

void ValidatorSize::setType(QMetaType::Type type)
{
    Q_D(ValidatorSize);
    d->type = type;
}

void ValidatorSize::setSize(double size)
{
    Q_D(ValidatorSize);
    d->size = size;
}
