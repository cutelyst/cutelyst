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

#include "validatorconfirmed_p.h"

using namespace Cutelyst;

ValidatorConfirmed::ValidatorConfirmed(const QString &field, const ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorConfirmedPrivate(field, messages))
{
}

ValidatorConfirmed::~ValidatorConfirmed()
{
}

ValidatorReturnType ValidatorConfirmed::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        const QString ofn = field() + QLatin1String("_confirmation");
        QString ofv = params.value(ofn);

        if (trimBefore()) {
            ofv = ofv.trimmed();
        }

        if (Q_UNLIKELY(v != ofv)) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorConfirmed: Failed to confirm the value in the field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        } else {
            result.value.setValue<QString>(v);
        }
    }

    return result;
}

QString ValidatorConfirmed::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorConfirmed", "Confirmation failed.");
    } else {
        error = c->translate("Cutelyst::ValidatorConfirmed", "The value in the “%1“ field has not been confirmed.").arg(_label);
    }
    return error;
}
