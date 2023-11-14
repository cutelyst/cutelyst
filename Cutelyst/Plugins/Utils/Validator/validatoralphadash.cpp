/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoralphadash_p.h"

using namespace Cutelyst;

const QRegularExpression ValidatorAlphaDashPrivate::regex{u"^[\\pL\\pM\\pN_-]+$"_qs};

ValidatorAlphaDash::ValidatorAlphaDash(const QString &field,
                                       bool asciiOnly,
                                       const ValidatorMessages &messages,
                                       const QString &defValKey)
    : ValidatorRule(*new ValidatorAlphaDashPrivate(field, asciiOnly, messages, defValKey))
{
}

ValidatorAlphaDash::~ValidatorAlphaDash() = default;

ValidatorReturnType ValidatorAlphaDash::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAlphaDash);

    const QString v = value(params);
    if (!v.isEmpty()) {
        if (Q_LIKELY(ValidatorAlphaDash::validate(v, d->asciiOnly))) {
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

bool ValidatorAlphaDash::validate(const QString &value, bool asciiOnly)
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
                   (uc <= ValidatorRulePrivate::ascii_9)) ||
                  (uc == ValidatorRulePrivate::ascii_dash) ||
                  (uc == ValidatorRulePrivate::ascii_underscore))) {
                valid = false;
                break;
            }
        }
    } else {
        valid = value.contains(ValidatorAlphaDashPrivate::regex);
    }
    return valid;
}

QString ValidatorAlphaDash::genericValidationError(Cutelyst::Context *c,
                                                   const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    Q_D(const ValidatorAlphaDash);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        if (d->asciiOnly) {
            //% "Must only contain alpha-numeric latin characters, dashes and underscores "
            //% "from the ASCII character encoding (a-z, A-Z, 0-9, _ and -)."
            return c->qtTrId("cutelyst-valalphadash-genvalerr-asciionly");
        } else {
            //% "Must only contain alpha-numeric characters, dashes and underscores."
            return c->qtTrId("cutelyst-valalphadash-genvalerr");
        }
    } else {
        if (d->asciiOnly) {
            //: %1 will be replaced by the field label
            //% "The text in the “%1” field must only contain alpha-numeric latin "
            //% "characters, dashes and underscores from the ASCII character encondig "
            //% "(a-z, A-Z, 0-9, _ and -)."
            return c->qtTrId("cutelyst-valalphadash-genvalerr-asciionly-label").arg(_label);
        } else {
            //: %1 will be replaced by the field label
            //% "The text in the “%1” field must only contain alpha-numeric "
            //% "characters, dashes and underscores."
            return c->qtTrId("cutelyst-valalphadash-genvalerr-label").arg(_label);
        }
    }
}
