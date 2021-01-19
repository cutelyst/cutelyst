/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
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

#include "validatornotin_p.h"

using namespace Cutelyst;

ValidatorNotIn::ValidatorNotIn(const QString &field, const QStringList &values, Qt::CaseSensitivity cs, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorNotInPrivate(field, values, cs, messages, defValKey))
{
}

ValidatorNotIn::~ValidatorNotIn()
{
}

ValidatorReturnType ValidatorNotIn::validate(Cutelyst::Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorNotIn);

    if (d->values.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorNotIn: The list of comparison values for the field %s at %s::%s is empty.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {
        const QString v = value(params);
        if (!v.isEmpty()) {
            if (d->values.contains(v, d->cs)) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorNotIn: Validation failed for field %s at %s::%s: \"%s\" is part of the list of not allowed comparison values.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(v));
            } else {
                result.value.setValue(v);
            }
        } else {
            defaultValue(c, &result, "ValidatorNotIn");
        }
    }

    return result;
}

QString ValidatorNotIn::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorNotIn", "Value is not allowed.");
    } else {
        error = c->translate("Cutelyst::ValidatorNotIn", "The value in the “%1” field is not allowed.").arg(_label);
    }
    return error;
}

QString ValidatorNotIn::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorNotIn", "The list of comparison values is empty.");
    } else {
        error = c->translate("Cutelyst::ValidatorNotIn", "The list of comparison values for the “%1” field is empty.").arg(_label);
    }
    return error;
}
