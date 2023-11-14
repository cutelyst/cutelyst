/*
 * SPDX-FileCopyrightText: (C) 2019-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorcharnotallowed_p.h"

using namespace Cutelyst;

ValidatorCharNotAllowed::ValidatorCharNotAllowed(const QString &field,
                                                 const QString &forbiddenChars,
                                                 const ValidatorMessages &messages,
                                                 const QString &defValKey)
    : ValidatorRule(*new ValidatorCharNotAllowedPrivate(field, forbiddenChars, messages, defValKey))
{
}

ValidatorCharNotAllowed::~ValidatorCharNotAllowed() = default;

bool ValidatorCharNotAllowed::validate(const QString &value,
                                       const QString &forbiddenChars,
                                       QChar *foundChar)
{
    bool valid = true;

    for (const QChar &forbiddenChar : forbiddenChars) {
        if (value.contains(forbiddenChar)) {
            valid = false;
            if (foundChar) {
                *foundChar = forbiddenChar;
            }
            break;
        }
    }

    return valid;
}

ValidatorReturnType ValidatorCharNotAllowed::validate(Context *c,
                                                      const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorCharNotAllowed);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(!d->forbiddenChars.isEmpty())) {
            QChar foundChar;
            if (Q_LIKELY(ValidatorCharNotAllowed::validate(v, d->forbiddenChars, &foundChar))) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c, foundChar);
            }
        } else {
            qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Empty validation data";
            result.errorMessage = validationDataError(c);
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorCharNotAllowed::genericValidationError(Context *c, const QVariant &errorData) const
{
    const QChar foundChar = errorData.toChar();
    Q_D(const ValidatorCharNotAllowed);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //: %1 will be replaced by string of forbidden chars, %2 by the forbidden
        //: char that has been found in the input
        //% "Must not contain the following characters: “%1”. But it contains the "
        //% "this illegal character: “%2”."
        return c->qtTrId("cutelyst-valcharnotallowed-genvalerr")
            .arg(d->forbiddenChars, QString(foundChar));
    } else {
        //: %1 will be replaced by the field label, %2 by the list of forbidden chars
        //: as a string, %3 will be the forbidden char that has been found
        //% "The text in the “%1“ field must not contain the following characters: "
        //% "“%2“. But it contains this illegal character: “%3”."
        return c->qtTrId("cutelyst-valcharnotallowed-genvalerr-label")
            .arg(_label, d->forbiddenChars, QString(foundChar));
    }
}

QString ValidatorCharNotAllowed::genericValidationDataError(Context *c,
                                                            const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "The list of illegal characters for this field is empty."
        return c->qtTrId("cutelyst-valcharnotallowed-genvaldataerr");
    } else {
        //% "The list of illegal characters for the “%1“ field is empty."
        return c->qtTrId("cutelyst-valcharnotallowed-genvaldataerr-label").arg(_label);
    }
}
