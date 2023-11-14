/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordigitsbetween_p.h"

using namespace Cutelyst;

ValidatorDigitsBetween::ValidatorDigitsBetween(const QString &field,
                                               const QVariant &min,
                                               const QVariant &max,
                                               const ValidatorMessages &messages,
                                               const QString &defValKey)
    : ValidatorRule(*new ValidatorDigitsBetweenPrivate(field, min, max, messages, defValKey))
{
}

ValidatorDigitsBetween::~ValidatorDigitsBetween() = default;

ValidatorReturnType ValidatorDigitsBetween::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDigitsBetween);

    const QString v = value(params);

    bool ok        = false;
    qsizetype _max = 0;
    qsizetype _min = d->extractSizeType(c, params, d->min, &ok);
    if (!ok) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote()
            << debugString(c) << "Invalid minimum length comparison data";
        return result;
    } else {
        _max = d->extractSizeType(c, params, d->max, &ok);
        if (!ok) {
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR).noquote()
                << debugString(c) << "Invalid maximum length comparison data";
            return result;
        }
    }

    if (_min > _max) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote()
            << debugString(c) << "Minimum comparison length" << _min << "is larger than"
            << "maximum comparison length" << _max;
        return result;
    }

    if (!v.isEmpty()) {

        if (Q_LIKELY(ValidatorDigitsBetween::validate(v, _min, _max))) {
            result.value.setValue(v);
        } else {
            result.errorMessage = validationError(c, QVariantList{_min, _max});
            qCDebug(C_VALIDATOR).noquote()
                << debugString(c) << "Length of" << v.length() << "is not between" << _min << "and"
                << _max << "and/or input value contains non-digit characters";
        }

    } else {
        defaultValue(c, &result);
    }

    return result;
}

bool ValidatorDigitsBetween::validate(const QString &value, int min, int max)
{
    for (const QChar &ch : value) {
        const ushort &uc = ch.unicode();
        if (!((uc >= ValidatorRulePrivate::ascii_0) && (uc <= ValidatorRulePrivate::ascii_9))) {
            return false;
        }
    }

    if ((value.length() < min) || (value.length() > max)) {
        return false;
    }

    return true;
}

QString ValidatorDigitsBetween::genericValidationError(Context *c, const QVariant &errorData) const
{
    const QVariantList list = errorData.toList();
    const QString min       = list.at(0).toString();
    const QString max       = list.at(1).toString();
    const QString _label    = label(c);

    if (_label.isEmpty()) {
        //% "Must contain between %1 and %2 digits."
        return c->qtTrId("cutelyst-valdigitsbetween-genvalerr").arg(min, max);
    } else {
        //: %1 will be replaced by the field label
        //% "The “%1” field must contain between %2 and %3 digits."
        return c->qtTrId("cutelyst-valdigitsbetween-genvalerr-label").arg(_label, min, max);
    }
}
