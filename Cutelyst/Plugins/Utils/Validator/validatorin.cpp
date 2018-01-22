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

#include "validatorin_p.h"

using namespace Cutelyst;

ValidatorIn::ValidatorIn(const QString &field, const QStringList &values, Qt::CaseSensitivity cs, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorInPrivate(field, values, cs, messages, defValKey))
{
}

ValidatorIn::~ValidatorIn()
{
}

ValidatorReturnType ValidatorIn::validate(Cutelyst::Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorIn);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (d->values.empty()) {
            qCWarning(C_VALIDATOR, "ValidatorIn: The list of comparison values for the field %s at %s::%s is empty.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            result.errorMessage = validationDataError(c);
        } else {
            if (d->values.contains(v, d->cs)) {
                result.value.setValue<QString>(v);
            } else {
                qCDebug(C_VALIDATOR, "ValidatorIn: Validation failed for field %s at %s::%s: \"%s\" is not part of the list of comparison values.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(v));
                result.errorMessage = validationError(c);
            }
        }
    } else {
        defaultValue(c, &result, "ValidatorIn");
    }

    return result;
}

QString ValidatorIn::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_D(const ValidatorIn);
    Q_UNUSED(errorData);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorIn", "Has to be one of the following: %1").arg(c->locale().createSeparatedList(d->values));
    } else {
        error = c->translate("Cutelyst::ValidatorIn", "The value in the “%1” field has to be one of the following: %2").arg(_label, c->locale().createSeparatedList(d->values));
    }
    return error;
}

QString ValidatorIn::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorIn", "The list of comparison values is empty.");
    } else {
        error = c->translate("Cutelyst::ValidatorIn", "The list of comparison values for the “%1” field is empty.").arg(_label);
    }
    return error;
}
