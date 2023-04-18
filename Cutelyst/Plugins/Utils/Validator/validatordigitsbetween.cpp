/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordigitsbetween_p.h"

using namespace Cutelyst;

ValidatorDigitsBetween::ValidatorDigitsBetween(const QString &field, const QVariant &min, const QVariant &max, const ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorDigitsBetweenPrivate(field, min, max, messages, defValKey))
{
}

ValidatorDigitsBetween::~ValidatorDigitsBetween()
{
}

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
        qCWarning(C_VALIDATOR, "ValidatorDigitsBetween: Invalid minimum validation length for field %s at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        return result;
    } else {
        _max = d->extractInt(c, params, d->max, &ok);
        if (!ok) {
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR, "ValidatorDigitsBetween: Invalid maximum validation length for field %s at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            return result;
        }
    }

    if (_min > _max) {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorDigitsBetween: Minimum length %i is larger than maximum length %i for field %s at %s::%s", _min, _max, qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        return result;
    }

    if (!v.isEmpty()) {

        if (Q_LIKELY(ValidatorDigitsBetween::validate(v, _min, _max))) {
            result.value.setValue(v);
        } else {
            result.errorMessage = validationError(c, QVariantList{_min, _max});
            qCDebug(C_VALIDATOR, "ValidatorDigitsBetween: Validation failed for value \"%s\" in field %s at %s::%s: length not between %i and %i and/or non-digit characters.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), _min, _max);
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
        if (!((uc > 47) && (uc < 58))) {
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
        error = c->translate("Cutelyst::ValidatorDigitsBetween", "Must contain between %1 and %2 digits.").arg(min, max);
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorDigitsBetween", "The “%1” field must contain between %2 and %3 digits.").arg(_label, min, max);
    }

    return error;
}
