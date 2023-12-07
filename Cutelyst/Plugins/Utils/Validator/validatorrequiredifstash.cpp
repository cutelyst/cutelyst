/*
 * SPDX-FileCopyrightText: (C) 2018-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredifstash_p.h"

using namespace Cutelyst;

ValidatorRequiredIfStash::ValidatorRequiredIfStash(const QString &field,
                                                   const QString &stashKey,
                                                   const QVariant &stashValues,
                                                   const ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorRequiredIfStashPrivate(field, stashKey, stashValues, messages))
{
}

ValidatorRequiredIfStash::~ValidatorRequiredIfStash() = default;

ValidatorReturnType ValidatorRequiredIfStash::validate(Context *c,
                                                       const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredIfStash);

    if (d->stashKey.isEmpty() || d->stashValues.isNull() || !d->stashValues.isValid()) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid validation data";
        return result;
    }

    const QString v   = value(params);
    const QVariant sv = c->stash(d->stashKey);
    bool isRequired   = false;

    if (d->stashValues.typeId() == QMetaType::QString) {

        const QVariant _stashValues = c->stash(d->stashValues.toString());

        if (_stashValues.typeId() == QMetaType::QStringList) {

            const auto str     = sv.toString();
            const auto strList = _stashValues.toStringList();
            isRequired         = strList.contains(str);

        } else if (_stashValues.typeId() == QMetaType::QVariantList) {

            const auto varList = _stashValues.toList();
            isRequired         = varList.contains(sv);

        } else {
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR).noquote().nospace()
                << debugString(c) << "Invalid validation data. The stash key \""
                << d->stashValues.toString()
                << "\" does not contain a QStringList or a QVariantList.";
            return result;
        }

    } else if (d->stashValues.typeId() == QMetaType::QStringList) {

        const auto str     = sv.toString();
        const auto strList = d->stashValues.toStringList();
        isRequired         = strList.contains(str);

    } else if (d->stashValues.typeId() == QMetaType::QVariantList) {

        const auto varList = d->stashValues.toList();
        isRequired         = varList.contains(sv);

    } else {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote() << debugString(c)
                                         << "Invalid validation data. "
                                            "stashValues has to be one of QString, "
                                            "QStringList or QVariantList.";
        return result;
    }

    if (isRequired && v.isEmpty()) {
        result.errorMessage = validationError(c);
        qCDebug(C_VALIDATOR).noquote().nospace()
            << debugString(c) << " The field is not present or empty but stash key \""
            << d->stashKey << "\" contains " << sv;
        return result;
    }

    if (!v.isEmpty()) {
        result.value.setValue(v);
    }

    return result;
}

QString ValidatorRequiredIfStash::genericValidationError(Context *c,
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
