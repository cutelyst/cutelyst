/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatornotin_p.h"

using namespace Cutelyst;

ValidatorNotIn::ValidatorNotIn(const QString &field,
                               const QVariant &values,
                               Qt::CaseSensitivity cs,
                               const Cutelyst::ValidatorMessages &messages,
                               const QString &defValKey)
    : ValidatorRule(*new ValidatorNotInPrivate(field, values, cs, messages, defValKey))
{
}

ValidatorNotIn::~ValidatorNotIn() = default;

ValidatorReturnType ValidatorNotIn::validate(Cutelyst::Context *c,
                                             const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorNotIn);

    const QString v = value(params);
    if (!v.isEmpty()) {
        QStringList vals;

        if (d->values.userType() == QMetaType::QStringList) {
            vals = d->values.toStringList();
        } else if (d->values.userType() == QMetaType::QString) {
            vals = c->stash(d->values.toString()).toStringList();
        }

        if (vals.empty()) {
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR).noquote()
                << debugString(c) << "The list of comparison values is empty";
        } else {
            if (vals.contains(v, d->cs)) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " \"" << v
                    << "\" is part of the list of not allowed values" << d->values;
            } else {
                result.value.setValue(v);
            }
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorNotIn::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "Value is not allowed."
        return c->qtTrId("cutelyst-valnotin-genvalerr");
    } else {
        //% "The value in the “%1” field is not allowed."
        return c->qtTrId("cutelyst-valnotin-genvalerr-label").arg(_label);
    }
}

QString ValidatorNotIn::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    // translation strings are defined in ValidatorIn

    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        return c->qtTrId("cutelyst-validator-genvaldataerr-empty-list");
    } else {
        return c->qtTrId("cutelyst-validator-genvaldataerr-empty-list-label").arg(_label);
    }
}
