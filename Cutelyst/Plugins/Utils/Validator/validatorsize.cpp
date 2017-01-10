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

bool ValidatorSize::validate()
{
    QString v = value();

    if (v.isEmpty()) {
        setError(ValidatorRule::NoError);
        return true;
    }

    Q_D(ValidatorSize);

    if (d->type == QMetaType::Int) {
        qlonglong val = v.toLongLong();
        qlonglong size = (qlonglong)d->size;
        if (val == size) {
            setError(ValidatorRule::NoError);
            return true;
        }
    } else if (d->type == QMetaType::UInt) {
        qulonglong val = v.toULongLong();
        qulonglong size = (qulonglong)d->size;
        if (val == size) {
            setError(ValidatorRule::NoError);
            return true;
        }
    } else if (d->type == QMetaType::Float) {
        double val = v.toDouble();
        if (val == d->size) {
            setError(ValidatorRule::NoError);
            return true;
        }
    } else if (d->type == QMetaType::QString) {
        int val = v.length();
        int size = (int)d->size;
        if (val == size) {
            setError(ValidatorRule::NoError);
            return true;
        }
    } else {
        setError(ValidatorRule::ValidationDataError);
    }

    return false;
}

QString ValidatorSize::genericErrorMessage() const
{
    Q_D(const ValidatorSize);

    if (d->type == QMetaType::Int || d->type == QMetaType::UInt) {
        return QStringLiteral("The value of the “%1” field has to be equal to %2.").arg(genericFieldName(),QString::number(d->size, 'f', 0));
    } else if (d->type == QMetaType::Float) {
        return QStringLiteral("The value of the “%1” field has to be equal to %2.").arg(genericFieldName(), QString::number(d->size));
    } else if (d->type == QMetaType::QString) {
        return QStringLiteral("The length of the “%1” field has to be equal to %2.").arg(genericFieldName(), QString::number(d->size, 'f', 0));
    } else {
        return QString();
    }
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
