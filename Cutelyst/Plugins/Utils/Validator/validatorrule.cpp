/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorrule_p.h"

#include <Cutelyst/Context>
#include <Cutelyst/ParamsMultiMap>

using namespace Cutelyst;

ValidatorRule::ValidatorRule(const QString &field,
                             const ValidatorMessages &messages,
                             const QString &defValKey,
                             QByteArrayView validatorName)
    : d_ptr(new ValidatorRulePrivate(field, messages, defValKey, validatorName))
{
}

ValidatorRule::ValidatorRule(ValidatorRulePrivate &dd)
    : d_ptr(&dd)
{
}

ValidatorRule::~ValidatorRule() = default;

QString ValidatorRule::field() const noexcept
{
    Q_D(const ValidatorRule);
    return d->field;
}

QString ValidatorRule::value(const Cutelyst::ParamsMultiMap &params) const
{
    Q_D(const ValidatorRule);

    if (!d->field.isEmpty() && !params.empty()) {
        if (d->trimBefore) {
            return params.value(d->field).trimmed();
        } else {
            return params.value(d->field);
        }
    }

    return {};
}

QString ValidatorRule::label(Context *c) const
{
    QString l;
    Q_D(const ValidatorRule);
    if (d->messages.label) {
        if (d->translationContext.size()) {
            l = c->translate(d->translationContext.data(), d->messages.label);
        } else {
            l = QString::fromUtf8(d->messages.label);
        }
    }
    return l;
}

QString ValidatorRule::validationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorRule);
    if (d->messages.validationError) {
        if (d->translationContext.size()) {
            error = c->translate(d->translationContext.data(), d->messages.validationError);
        } else {
            error = QString::fromUtf8(d->messages.validationError);
        }
    } else {
        error = genericValidationError(c, errorData);
    }
    return error;
}

QString ValidatorRule::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (!_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRule",
                             "The input data in the “%1” field is not acceptable.")
                    .arg(_label);
    } else {
        error = c->translate("Cutelyst::ValidatorRule", "The input data is not acceptable.");
    }
    return error;
}

QString ValidatorRule::parsingError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_D(const ValidatorRule);
    Q_UNUSED(errorData)
    if (d->messages.parsingError) {
        if (d->translationContext.size()) {
            error = c->translate(d->translationContext.data(), d->messages.parsingError);
        } else {
            error = QString::fromUtf8(d->messages.parsingError);
        }
    } else {
        error = genericParsingError(c, errorData);
    }
    return error;
}

QString ValidatorRule::genericParsingError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (!_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRule",
                             "The input data in the “%1“ field could not be parsed.")
                    .arg(_label);
    } else {
        error = c->translate("Cutelyst::ValidatorRule", "The input data could not be parsed.");
    }
    return error;
}

QString ValidatorRule::validationDataError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_D(const ValidatorRule);
    Q_UNUSED(errorData)
    if (d->messages.validationDataError) {
        if (d->translationContext.size()) {
            error = c->translate(d->translationContext.data(), d->messages.validationDataError);
        } else {
            error = QString::fromUtf8(d->messages.validationDataError);
        }
    } else {
        error = genericValidationDataError(c, errorData);
    }
    return error;
}

QString ValidatorRule::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (!_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRule",
                             "Missing or invalid validation data for the “%1” field.")
                    .arg(_label);
    } else {
        error = c->translate("Cutelyst::ValidatorRule", "Missing or invalid validation data.");
    }
    return error;
}

void ValidatorRule::defaultValue(Context *c, ValidatorReturnType *result) const
{
    Q_ASSERT_X(c, "getting default value", "invalid context object");
    Q_ASSERT_X(result, "getting default value", "invalid result object");
    Q_D(const ValidatorRule);
    if (!d->defValKey.isEmpty() && c->stash().contains(d->defValKey)) {
        result->value.setValue(c->stash(d->defValKey));
        qCDebug(C_VALIDATOR).noquote().nospace()
            << d->validatorName << ": Using default value " << result->value << " for field \""
            << field() << "\" at " << c->controllerName() << "::" << c->actionName();
    }
}

QString ValidatorRule::debugString(Context *c) const
{
    Q_D(const ValidatorRule);
    return QString::fromLatin1(d->validatorName) +
           QLatin1String(": Validation failed for field \"") + field() + QLatin1String("\" at ") +
           c->controllerName() + QLatin1String("::") + c->actionName() + QLatin1Char(':');
}

bool ValidatorRule::trimBefore() const noexcept
{
    Q_D(const ValidatorRule);
    return d->trimBefore;
}

void ValidatorRule::setTrimBefore(bool trimBefore) noexcept
{
    Q_D(ValidatorRule);
    d->trimBefore = trimBefore;
}

void ValidatorRule::setTranslationContext(QLatin1String trContext) noexcept
{
    Q_D(ValidatorRule);
    d->translationContext = trContext;
}
