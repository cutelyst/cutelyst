/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwithout_p.h"

using namespace Cutelyst;

ValidatorRequiredWithout::ValidatorRequiredWithout(const QString &field,
                                                   const QStringList &otherFields,
                                                   const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithoutPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithout::~ValidatorRequiredWithout() = default;

ValidatorReturnType ValidatorRequiredWithout::validate(Context *c,
                                                       const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithout);

    if (d->otherFields.isEmpty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << "Invalid validation data";
    } else {

        bool otherMissing = false;

        const QStringList ofc = d->otherFields;

        QString otherField;
        for (const QString &other : ofc) {
            if (!params.contains(other)) {
                otherField   = other;
                otherMissing = true;
                break;
            }
        }

        const QString v = value(params);

        if (otherMissing) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " The field is not present or empty but the field \""
                    << otherField << "\" is not present";
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithout::genericValidationError(Context *c,
                                                         const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRequiredWithout", "This is required.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRequiredWithout", "The “%1” field is required.")
                    .arg(_label);
    }
    return error;
}
