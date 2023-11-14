/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredif_p.h"

using namespace Cutelyst;

ValidatorRequiredIf::ValidatorRequiredIf(const QString &field,
                                         const QString &otherField,
                                         const QStringList &otherValues,
                                         const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredIfPrivate(field, otherField, otherValues, messages))
{
}

ValidatorRequiredIf::~ValidatorRequiredIf() = default;

ValidatorReturnType ValidatorRequiredIf::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredIf);

    if (d->otherField.isEmpty() || d->otherValues.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid validation data";
    } else {
        const QString v = value(params);
        const QString ov =
            trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);
        if (d->otherValues.contains(ov)) {
            if (v.isEmpty()) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " The field is not present or empty but \""
                    << d->otherField << "\" contains " << ov;
            } else {
                result.value.setValue(v);
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredIf::genericValidationError(Context *c, const QVariant &errorData) const
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
