/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwith_p.h"

using namespace Cutelyst;

ValidatorRequiredWith::ValidatorRequiredWith(const QString &field,
                                             const QStringList &otherFields,
                                             const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWith::~ValidatorRequiredWith() = default;

ValidatorReturnType ValidatorRequiredWith::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWith);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid validation data";
    } else {
        const QString v = value(params);

        auto it = std::ranges::find_if(
            d->otherFields, [&params](const QString &other) { return params.contains(other); });

        if (it != d->otherFields.end()) { // Contains other
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote()
                    << debugString(c) << "The field is not present or empty but the field \"" << *it
                    << "\" is present";
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWith::genericValidationError(Context *c, const QVariant &errorData) const
{
    // translation strings are defined in ValidatorRequired
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        return c->qtTrId("cutelyst-validator-genvalerr-req");
    } else {
        return c->qtTrId("cutelyst-validator-genvalerr-req-label").arg(_label);
    }
}
