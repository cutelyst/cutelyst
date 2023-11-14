/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwithall_p.h"

using namespace Cutelyst;

ValidatorRequiredWithAll::ValidatorRequiredWithAll(const QString &field,
                                                   const QStringList &otherFields,
                                                   const ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithAllPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithAll::~ValidatorRequiredWithAll() = default;

ValidatorReturnType ValidatorRequiredWithAll::validate(Context *c,
                                                       const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithAll);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid validation data";
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
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote() << debugString(c)
                                               << "The field is not present or empty but all other "
                                                  "required fields are present";
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithAll::genericValidationError(Context *c,
                                                         const QVariant &errorData) const
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
