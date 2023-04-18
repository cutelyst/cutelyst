/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoralphanum_p.h"

#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlphaNum::ValidatorAlphaNum(const QString &field, bool asciiOnly, const ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorAlphaNumPrivate(field, asciiOnly, messages, defValKey))
{
}

ValidatorAlphaNum::~ValidatorAlphaNum()
{
}

ValidatorReturnType ValidatorAlphaNum::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlphaNum);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlphaNum::validate(v, d->asciiOnly))) {
            result.value.setValue(v);
        } else {
            qCDebug(C_VALIDATOR, "ValidatorAlphaNum: Validation failed for field %s at %s::%s: %s contains characters that are not allowed.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(v));
            result.errorMessage = validationError(c);
        }

    } else {
        defaultValue(c, &result, "ValidatorAlphaNum");
    }

    return result;
}

bool ValidatorAlphaNum::validate(const QString &value, bool asciiOnly)
{
    bool valid = true;
    if (asciiOnly) {
        for (const QChar &ch : value) {
            const ushort &uc = ch.unicode();
            if (!(((uc > 64) && (uc < 91)) || ((uc > 96) && (uc < 123)) || ((uc > 47) && (uc < 58)))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(QRegularExpression(QStringLiteral("^[\\pL\\pM\\pN]+$")));
    }
    return valid;
}

QString ValidatorAlphaNum::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorAlphaNum);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        if (d->asciiOnly) {
            error = c->translate("Cutelyst::ValidatorAlphaNum", "Must only contain alpha-numeric latin characters.");
        } else {
            error = c->translate("Cutelyst::ValidatorAlphaNum", "Must only contain alpha-numeric characters.");
        }
    } else {
        if (d->asciiOnly) {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorAlphaNum", "The text in the “%1” field must only contain alpha-numeric latin characters.").arg(_label);
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorAlphaNum", "The text in the “%1” field must only contain alpha-numeric characters.").arg(_label);
        }
    }
    return error;
}
