/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validator_p.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/request.h>

#include <QLoggingCategory>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

Q_LOGGING_CATEGORY(C_VALIDATOR, "cutelyst.utils.validator", QtWarningMsg)

Validator::Validator(const char *translationContext)
    : d_ptr(new ValidatorPrivate(translationContext))
{
}

Validator::Validator(std::initializer_list<ValidatorRule *> validators,
                     const char *translationContext)
    : d_ptr(new ValidatorPrivate(validators, translationContext))
{
}

Validator::~Validator() = default;

void Validator::clear()
{
    Q_D(Validator);
    if (!d->validators.empty()) {
        qDeleteAll(d->validators.begin(), d->validators.end());
        d->validators.clear();
    }
}

Cutelyst::ValidatorResult Validator::validate(Context *c, ValidatorFlags flags) const
{
    ValidatorResult result;

    Q_ASSERT(c);

    ParamsMultiMap params;
    if (flags.testFlag(BodyParamsOnly)) {
        params = c->req()->bodyParameters();
    } else if (flags.testFlag(QueryParamsOnly)) {
        params = c->req()->queryParameters();
    } else {
        params = c->req()->queryParameters();
        params.unite(c->req()->bodyParameters());
    }

    result = validate(c, params, flags);

    return result;
}

ValidatorResult
    Validator::validate(Context *c, const ParamsMultiMap &params, ValidatorFlags flags) const
{
    ValidatorResult result;

    Q_ASSERT(c);
    Q_D(const Validator);

    if (d->validators.empty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty validator list.";
        return result;
    }

    if (params.empty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty parameters.";
    }

    const bool stopOnFirstError = flags.testFlag(StopOnFirstError);
    const bool noTrimming       = flags.testFlag(NoTrimming);

    for (ValidatorRule *v : d->validators) {

        if (noTrimming) {
            v->setTrimBefore(false);
        }

        const ValidatorReturnType singleResult = v->validate(c, params);

        if (singleResult.extra.isValid()) {
            result.addExtra(v->field(), singleResult.extra);
        }

        if (singleResult) {
            result.addValue(v->field(), singleResult.value);
        } else {
            result.addError(v->field(), singleResult.errorMessage);
            if (stopOnFirstError) {
                break;
            }
        }
    }

    if (!result && flags.testFlag(FillStashOnError)) {
        c->setStash(u"validationErrorStrings"_s, result.errorStrings());
        c->setStash(u"validationErrors"_s, QVariant::fromValue(result.errors()));

        for (const auto &[key, value] : params.asKeyValueRange()) {
            if (!key.contains(u"password"_s, Qt::CaseInsensitive)) {
                c->setStash(key, value);
            }
        }
    }

    return result;
}

AwaitedValidatorResult Validator::coValidate(Context *c, ValidatorFlags flags) const
{
    AwaitedValidatorResult avr{c};
    auto cb = avr.callback;

    Q_D(const Validator);

    ParamsMultiMap params;
    if (flags.testFlag(BodyParamsOnly)) {
        params = c->req()->bodyParameters();
    } else if (flags.testFlag(QueryParamsOnly)) {
        params = c->req()->queryParameters();
    } else {
        params = c->req()->queryParameters();
        params.unite(c->req()->bodyParameters());
    }

    auto async = new AsyncValidator{c};
    QObject::connect(async,
                     &AsyncValidator::finished,
                     [cb](const Cutelyst::ValidatorResult &result) { cb(result); });
    async->start(d->validators, flags, params);

    return avr;
}

AwaitedValidatorResult
    Validator::coValidate(Context *c, const ParamsMultiMap &params, ValidatorFlags flags) const
{
    AwaitedValidatorResult avr{c};
    auto cb = avr.callback;

    Q_D(const Validator);

    auto async = new AsyncValidator{c};
    QObject::connect(async,
                     &AsyncValidator::finished,
                     [cb](const Cutelyst::ValidatorResult &result) { cb(result); });
    async->start(d->validators, flags, params);

    return avr;
}

void Validator::addValidator(ValidatorRule *v)
{
    Q_D(Validator);
    v->setTranslationContext(d->translationContext);
    d->validators.push_back(v);
}

void Validator::loadTranslations(Application *app)
{
    app->loadTranslations(u"plugin_utils_validator"_s);
}

void AsyncValidator::start(const std::vector<ValidatorRule *> &validators,
                           Validator::ValidatorFlags flags,
                           const ParamsMultiMap &params)
{
    if (validators.empty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty validator list.";
        Q_EMIT finished(m_result);
        return;
    }

    if (params.empty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty parameters.";
    }

    m_params = params;

    for (auto validator : validators) {
        m_validators.enqueue(validator);
    }

    m_stopOnFirstError = flags.testFlag(Validator::StopOnFirstError);
    m_noTrimming       = flags.testFlag(Validator::NoTrimming);
    m_fillStashOnError = flags.testFlag(Validator::FillStashOnError);

    QMetaObject::invokeMethod(this, "doValidation", Qt::DirectConnection);
}

void AsyncValidator::doValidation()
{
    if (m_validators.empty() || m_cancelValidation) {
        Q_EMIT finished(m_result);
        return;
    }

    auto v = m_validators.dequeue();

    if (m_context.isNull()) {
        qCCritical(C_VALIDATOR)
            << "Cutelyst context object was destroyed while performing validation";
        m_result.addError(
            v->field(),
            QStringLiteral("Cutelyst context object was destroyed while performing validation."));
        Q_EMIT finished(m_result);
        return;
    }

    v->validateCb(m_context.get(), m_params, [this, v](const ValidatorReturnType &singleResult) {
        if (singleResult.extra.isValid()) {
            m_result.addExtra(v->field(), singleResult.extra);
        }

        if (singleResult) {
            m_result.addValue(v->field(), singleResult.value);
        } else {
            m_result.addError(v->field(), singleResult.errorMessage);
            if (m_stopOnFirstError) {
                m_cancelValidation = true;
            }
        }
        doValidation();
    });
}
