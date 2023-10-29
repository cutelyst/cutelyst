/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
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

    bool ok  = false;
    int _max = 0;
    int _min = d->extractInt(c, params, d->min, &ok);
    if (!ok) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote()
                << "ValidatorDigitsBetween: Invalid minimum validation length for field"
                << field() << "at" << caName(c);
        return result;
    } else {
        _max = d->extractInt(c, params, d->max, &ok);
        if (!ok) {
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR).noquote()
                    << "ValidatorBetween: Invalid maximum validation length for field"
                    << field() << "at" << caName(c);
            return result;
        }
    }

    if (_min > _max) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR).noquote()
                << "ValidatorDigitsBetween: Minimum length" << _min << "is larger than"
                << "maximum lenth" << _max << "for field" << field() << "at" << caName(c);
        return result;
    }

    if (!v.isEmpty()) {

        if (Q_LIKELY(ValidatorDigitsBetween::validate(v, _min, _max))) {
            result.value.setValue(v);
        } else {
            result.errorMessage = validationError(c, QVariantList{_min, _max});
            qCDebug(C_VALIDATOR).noquote().nospace()
                    << "ValidatorBetween: Validation failed for value \"" << v << "\" in field "
                    << field() << " at " << caName(c) << ": length not between " << _min << " and "
                    << _max << " and/or non-digit characters in the input value";
        }

    } else {
        defaultValue(c, &result, "ValidatorDigitsBetween");
    }

    return result;
}

bool ValidatorDigitsBetween::validate(const QString &value, int min, int max)
{
    bool valid = true;

    for (const QChar &ch : value) {
        const ushort &uc = ch.unicode();
        if (!((uc >= ValidatorRulePrivate::ascii_0) && (uc <= ValidatorRulePrivate::ascii_9))) {
            valid = false;
            break;
        }
    }

    if (valid && ((value.length() < min) || (value.length() > max))) {
        valid = false;
    }

    return valid;
}

QString ValidatorDigitsBetween::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    const QVariantList list = errorData.toList();
    const QString min       = list.at(0).toString();
    const QString max       = list.at(1).toString();
    const QString _label    = label(c);

    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorDigitsBetween",
                             "Must contain between %1 and %2 digits.")
                    .arg(min, max);
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorDigitsBetween",
                             "The “%1” field must contain between %2 and %3 digits.")
                    .arg(_label, min, max);
    }

    return error;
}
