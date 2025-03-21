﻿/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
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
    d->params.clear();
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
