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

#include "validatorrequiredwithall_p.h"

using namespace Cutelyst;

ValidatorRequiredWithAll::ValidatorRequiredWithAll(const QString &field, const QStringList &otherFields, const ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorRequiredWithAllPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithAll::~ValidatorRequiredWithAll()
{
}

ValidatorReturnType ValidatorRequiredWithAll::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithAll);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRequiredWithAll: invalid validation data for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    } else {

        bool containsAll = true;

        const QStringList ofc = d->otherFields;

        for (const QString &other : ofc) {
            if (!params.contains(other)) {
                containsAll = false;
                break;
            }
        }

        const QString v = value(params);

        if (containsAll) {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRequiredWithAll: Validation failed for field %s at %s::%s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue<QString>(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithAll::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    const QString _label = label(c);
    Q_UNUSED(errorData)
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWithAll", "This is required.");
    } else {
        error = c->translate("Cutelyst::ValidatorRequiredWithAll", "You must fill in the “%1” field.").arg(_label);
    }
    return error;
}
