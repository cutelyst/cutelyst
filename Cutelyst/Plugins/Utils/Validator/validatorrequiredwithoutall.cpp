/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredwithoutall_p.h"

using namespace Cutelyst;

ValidatorRequiredWithoutAll::ValidatorRequiredWithoutAll(
    const QString &field,
    const QStringList &otherFields,
    const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredWithoutAllPrivate(field, otherFields, messages))
{
}

ValidatorRequiredWithoutAll::~ValidatorRequiredWithoutAll() = default;

ValidatorReturnType ValidatorRequiredWithoutAll::validate(Context *c,
                                                          const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredWithoutAll);

    if (d->otherFields.empty()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << "Invalid validation data";
    } else {

        const QStringList ofc = d->otherFields;

        bool withoutAll = true;

        for (const QString &other : ofc) {
            if (params.contains(other)) {
                withoutAll = false;
                break;
            }
        }

        const QString v = value(params);

        if (withoutAll) {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote() << debugString(c)
                                               << "The field is not present or empty and all of "
                                                  "the other fields are not present";
            }
        } else {
            if (!v.isEmpty()) {
                result.value.setValue(v);
            }
        }
    }

    return result;
}

QString ValidatorRequiredWithoutAll::genericValidationError(Context *c,
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
