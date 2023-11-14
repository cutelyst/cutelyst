/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoralpha_p.h"

using namespace Cutelyst;

const QRegularExpression ValidatorAlphaPrivate::regex{u"^[\\pL\\pM]+$"_qs};

ValidatorAlpha::ValidatorAlpha(const QString &field,
                               bool asciiOnly,
                               const Cutelyst::ValidatorMessages &messages,
                               const QString &defValKey)
    : ValidatorRule(*new ValidatorAlphaPrivate(field, asciiOnly, messages, defValKey))
{
}

ValidatorAlpha::~ValidatorAlpha() = default;

ValidatorReturnType ValidatorAlpha::validate(Cutelyst::Context *c,
                                             const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlpha);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlpha::validate(v, d->asciiOnly))) {
            result.value.setValue(v);
        } else {
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " \"" << v << "\" contains character that are not allowed";
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

bool ValidatorAlpha::validate(const QString &value, bool asciiOnly)
{
    bool valid = true;

    if (asciiOnly) {
        for (const QChar &ch : value) {
            const ushort &uc = ch.unicode();
            if (!(((uc >= ValidatorRulePrivate::ascii_A) &&
                   (uc <= ValidatorRulePrivate::ascii_Z)) ||
                  ((uc >= ValidatorRulePrivate::ascii_a) &&
                   (uc <= ValidatorRulePrivate::ascii_z)))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(ValidatorAlphaPrivate::regex);
    }

    return valid;
}

QString ValidatorAlpha::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    Q_D(const ValidatorAlpha);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        if (d->asciiOnly) {
            //% "Must only contain alphabetical characters from the ASCII character "
            //% "encoding (a-z and A-Z)."
            return c->qtTrId("cutelyst-valalpha-genvalerr-asciionly");
        } else {
            //% "Must only contain alphabetical characters."
            return c->qtTrId("cutelyst-valalpha-genvalerr");
        }
    } else {
        if (d->asciiOnly) {
            //: %1 will be replaced by the field label
            //% "The text in the “%1” field must only contain alphabetical characters "
            //% "from the ASCII character encoding (a-z and A-Z)."
            return c->qtTrId("cutelyst-valalpha-genvalerr-asciionly-label").arg(_label);
        } else {
            //: %1 will be replaced by the field label
            //% "The text in the “%1” field must only contain alphabetical characters."
            return c->qtTrId("cutelyst-valalpha-genvalerr-label").arg(_label);
        }
    }
}
