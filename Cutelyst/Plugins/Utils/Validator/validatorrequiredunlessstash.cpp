/*
 * SPDX-FileCopyrightText: (C) 2018-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrequiredunlessstash_p.h"

using namespace Cutelyst;

ValidatorRequiredUnlessStash::ValidatorRequiredUnlessStash(const QString &field,
                                                           const QString &stashKey,
                                                           const QVariant &stashValues,
                                                           const ValidatorMessages &messages)
    : ValidatorRule(
          *new ValidatorRequiredUnlessStashPrivate(field, stashKey, stashValues, messages))
{
}

ValidatorRequiredUnlessStash::~ValidatorRequiredUnlessStash() = default;

ValidatorReturnType ValidatorRequiredUnlessStash::validate(Context *c,
                                                           const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRequiredUnlessStash);

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

            const auto str    = sv.toString();
            const auto strLst = _stashValues.toStringList();
            isRequired        = !strLst.contains(str);

        } else if (_stashValues.typeId() == QMetaType::QVariantList) {

            const auto varLst = _stashValues.toList();
            isRequired        = !varLst.contains(sv);

        } else {
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR).noquote().nospace()
                << debugString(c) << "Invalid validation data. The stash key \""
                << d->stashValues.toString()
                << "\" does not contain a QStringList or a QVariantList.";
            return result;
        }

    } else if (d->stashValues.typeId() == QMetaType::QStringList) {

        const auto str    = sv.toString();
        const auto strLst = d->stashValues.toStringList();
        isRequired        = !strLst.contains(str);

    } else if (d->stashValues.typeId() == QMetaType::QVariantList) {

        const auto varList = d->stashValues.toList();
        isRequired         = !varList.contains(sv);

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
            << d->stashKey << "\" does not contain " << sv;
        return result;
    }

    if (!v.isEmpty()) {
        result.value.setValue(v);
    }

    return result;
}

void ValidatorRequiredUnlessStash::validateCb(Context *c,
                                              const ParamsMultiMap &params,
                                              ValidatorRtFn cb) const
{
    cb(validate(c, params));
}

QString ValidatorRequiredUnlessStash::genericValidationError(Context *c,
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
