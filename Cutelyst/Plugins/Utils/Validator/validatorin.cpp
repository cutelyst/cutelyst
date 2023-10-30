/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorin_p.h"

using namespace Cutelyst;

ValidatorIn::ValidatorIn(const QString &field,
                         const QVariant &values,
                         Qt::CaseSensitivity cs,
                         const Cutelyst::ValidatorMessages &messages,
                         const QString &defValKey)
    : ValidatorRule(*new ValidatorInPrivate(field, values, cs, messages, defValKey))
{
}

ValidatorIn::~ValidatorIn() = default;

ValidatorReturnType ValidatorIn::validate(Cutelyst::Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorIn);

    const QString v = value(params);
    if (!v.isEmpty()) {
        QStringList vals;

        if (d->values.userType() == QMetaType::QStringList) {
            vals = d->values.toStringList();
        } else if (d->values.userType() == QMetaType::QString) {
            vals = c->stash(d->values.toString()).toStringList();
        }

        if (vals.empty()) {
            qCWarning(C_VALIDATOR).noquote()
                    << "ValidatorIn: The list of comparison values for the field"
                    << field() << "at" << caName(c) << "is empty";
        } else {
            if (vals.contains(v, d->cs)) {
                result.value.setValue(v);
            } else {
                qCDebug(C_VALIDATOR).noquote().nospace()
                        << "ValidatorIn: Validation failed for field " << field()
                        << " at " << caName(c) << ": \"" << v << "\" is not part of the comparison value list";
                result.errorMessage = validationError(c, vals);
            }
        }
    } else {
        defaultValue(c, &result, "ValidatorIn");
    }

    return result;
}

QString ValidatorIn::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    const QStringList vals = errorData.toStringList();
    const QString _label   = label(c);
    if (_label.isEmpty()) {
        //: %1 will be replaced by a comma separated list of allowed values
        error = c->translate("Cutelyst::ValidatorIn", "Has to be one of the following values: %1")
                    .arg(c->locale().createSeparatedList(vals));
    } else {
        //: %1 will be replaced by the field label, %2 will be replaced by a comma separated list of
        //: allowed values
        error =
            c->translate("Cutelyst::ValidatorIn",
                         "The value in the “%1” field has to be one of the following values: %2")
                .arg(_label, c->locale().createSeparatedList(vals));
    }
    return error;
}

QString ValidatorIn::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorIn", "The list of comparison values is empty.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorIn",
                             "The list of comparison values for the “%1” field is empty.")
                    .arg(_label);
    }
    return error;
}
