/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoralpha_p.h"

#include <QRegularExpression>

using namespace Cutelyst;

ValidatorAlpha::ValidatorAlpha(const QString &field, bool asciiOnly, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorAlphaPrivate(field, asciiOnly, messages, defValKey))
{
}

ValidatorAlpha::~ValidatorAlpha()
{
}

ValidatorReturnType ValidatorAlpha::validate(Cutelyst::Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlpha);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlpha::validate(v, d->asciiOnly))) {
            result.value.setValue(v);
        } else {
            qCDebug(C_VALIDATOR, "ValidatorAlhpa: Validation failed for field %s at %s::%s: %s contains characters that are not allowed.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(v));
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorAlpha");
    }

    return result;
}

bool ValidatorAlpha::validate(const QString &value, bool asciiOnly)
{
    bool valid = true;

    if (asciiOnly) {
        for (const QChar &ch : value) {
            const ushort &uc = ch.unicode();
            if (!(((uc > 64) && (uc < 91)) || ((uc > 96) && (uc < 123)))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(QRegularExpression(QStringLiteral("^[\\pL\\pM]+$")));
    }

    return valid;
}

QString ValidatorAlpha::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorAlpha);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        if (d->asciiOnly) {
            error = c->translate("Cutelyst::ValidatorAlhpa", "Must only contain alphabetical latin characters.");
        } else {
            error = c->translate("Cutelyst::ValidatorAlhpa", "Must only contain alphabetical characters.");
        }
    } else {
        if (d->asciiOnly) {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorAlhpa", "The text in the “%1” field must only contain alphabetical latin characters.").arg(_label);
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorAlhpa", "The text in the “%1” field must only contain alphabetical characters.").arg(_label);
        }
    }
    return error;
}
