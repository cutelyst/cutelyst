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
                    << debugString(c)
                    << "The list of comparison values is empty";
        } else {
            if (vals.contains(v, d->cs)) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote().nospace()
                        << debugString(c)
                        << " \"" << v << "\" is part of the list of not allowed values" << d->values;
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
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorNotIn", "Value is not allowed.");
    } else {
        error =
            c->translate("Cutelyst::ValidatorNotIn", "The value in the “%1” field is not allowed.")
                .arg(_label);
    }
    return error;
}

QString ValidatorNotIn::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorNotIn", "The list of comparison values is empty.");
    } else {
        error = c->translate("Cutelyst::ValidatorNotIn",
                             "The list of comparison values for the “%1” field is empty.")
                    .arg(_label);
    }
    return error;
}
