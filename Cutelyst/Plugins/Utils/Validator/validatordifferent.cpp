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

#include "validatordifferent_p.h"

using namespace Cutelyst;

ValidatorDifferent::ValidatorDifferent(const QString &field, const QString &other, const char *otherLabel, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorDifferentPrivate(field, other, otherLabel, messages))
{
}

ValidatorDifferent::~ValidatorDifferent()
{
}

ValidatorReturnType ValidatorDifferent::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDifferent);

    const QString v = value(params);
    const QString o = trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);

    if (!v.isEmpty()) {
        if ((v == o)) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorDifferent: Validation failed for value %s in field %s at %s::%s: the value in the %s field is not different.",
                    qPrintable(v),
                    qPrintable(field()),
                    qPrintable(c->controllerName()),
                    qPrintable(c->actionName()),
                    qPrintable(d->otherField));
        } else {
            result.value.setValue<QString>(v);
        }
    }

    return result;
}

QString ValidatorDifferent::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorDifferent);

    Q_UNUSED(errorData);

    const QString _label = label(c);
    const QString _otherLabel = d->otherLabel ? c->translate(d->translationContext.data(), d->otherLabel) : QString();

    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorDifferent", "Has to be different from the value in the “%1” field.").arg(!_otherLabel.isEmpty() ? _otherLabel : d->otherField);
    } else {
        error = c->translate("Cutelyst::ValidatorDifferent", "The value in the field “%1” has to be different from the value in the field “%2“.").arg(_label, !_otherLabel.isEmpty() ? _otherLabel : d->otherField);
    }

    return error;
}

