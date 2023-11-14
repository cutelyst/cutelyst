/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoralphanum_p.h"

using namespace Cutelyst;

const QRegularExpression ValidatorAlphaNumPrivate::regex{u"^[\\pL\\pM\\pN]+$"_qs};

ValidatorAlphaNum::ValidatorAlphaNum(const QString &field,
                                     bool asciiOnly,
                                     const ValidatorMessages &messages,
                                     const QString &defValKey)
    : ValidatorRule(*new ValidatorAlphaNumPrivate(field, asciiOnly, messages, defValKey))
{
}

ValidatorAlphaNum::~ValidatorAlphaNum() = default;

ValidatorReturnType ValidatorAlphaNum::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlphaNum);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlphaNum::validate(v, d->asciiOnly))) {
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

bool ValidatorAlphaNum::validate(const QString &value, bool asciiOnly)
{
    bool valid = true;
    if (asciiOnly) {
        for (const QChar &ch : value) {
            const ushort &uc = ch.unicode();
            if (!(((uc >= ValidatorRulePrivate::ascii_A) &&
                   (uc <= ValidatorRulePrivate::ascii_Z)) ||
                  ((uc >= ValidatorRulePrivate::ascii_a) &&
                   (uc <= ValidatorRulePrivate::ascii_z)) ||
                  ((uc >= ValidatorRulePrivate::ascii_0) &&
                   (uc <= ValidatorRulePrivate::ascii_9)))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(ValidatorAlphaNumPrivate::regex);
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
            //% "Must only contain alpha-numeric latin characters from the ASCII "
            //% "character encondig (a-z, A-Z and 0-9)."
            return c->qtTrId("cutelyst-valalphanum-genvalerr-asciionly");
        } else {
            //% "Must only contain alpha-numeric characters."
            return c->qtTrId("cutelyst-valalphanum-genvalerr");
        }
    } else {
        if (d->asciiOnly) {
            //: %1 will be replaced by the field label
            //% "The text in the “%1” field must only contain alpha-numeric latin characters "
            //% "from the ASCII character encondig (a-z, A-Z and 0-9)."
            return c->qtTrId("cutelyst-valalphanum-genvalerr-asciionly-label").arg(_label);
        } else {
            //: %1 will be replaced by the field label
            //% "The text in the “%1” field must only contain alpha-numeric characters."
            return c->qtTrId("cutelyst-valalphanum-genvalerr-label").arg(_label);
        }
    }
    return error;
}
