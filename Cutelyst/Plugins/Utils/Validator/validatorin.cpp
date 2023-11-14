/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
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
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR).noquote()
                << debugString(c) << "The list of comparison values is emtpy";
        } else {
            if (vals.contains(v, d->cs)) {
                result.value.setValue(v);
            } else {
                qCDebug(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " \"" << v << "\" is not part of the comparison list "
                    << vals;
                result.errorMessage = validationError(c, vals);
            }
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorIn::genericValidationError(Context *c, const QVariant &errorData) const
{
    const QStringList vals = errorData.toStringList();
    const QString _label   = label(c);
    if (_label.isEmpty()) {
        //: %1 will be replaced by a comma separated list of allowed values
        //% "Has to be one of the following values: %1"
        return c->qtTrId("cutelyst-valin-genvalerr").arg(c->locale().createSeparatedList(vals));
    } else {
        //: %1 will be replaced by the field label, %2 will be replaced by a comma
        //: separated list of allowed values
        //% "The value in the “%1” field has to be one of the following values: %2"
        return c->qtTrId("cutelyst-valin-genvalerr-label")
            .arg(_label, c->locale().createSeparatedList(vals));
    }
}

QString ValidatorIn::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "The list of comparison values is empty."
        return c->qtTrId("cutelyst-validator-genvaldataerr-empty-list");
    } else {
        //: %1 will be replaced by the field label
        //% "The list of comparison values for the “%1” field is empty."
        return c->qtTrId("cutelyst-validator-genvaldataerr-empty-list-label").arg(_label);
    }
}
