/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validator_p.h"

#include <Cutelyst/application.h>
#include <Cutelyst/context.h>
#include <Cutelyst/request.h>

#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_VALIDATOR, "cutelyst.utils.validator", QtWarningMsg)

Validator::Validator(QLatin1String translationContext)
    : d_ptr(new ValidatorPrivate(translationContext))
{
}

#ifdef Q_COMPILER_INITIALIZER_LISTS
Validator::Validator(std::initializer_list<ValidatorRule *> validators, QLatin1String translationContext)
    : d_ptr(new ValidatorPrivate(validators, translationContext))
{
}
#endif

Validator::~Validator()
{
}

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

ValidatorResult Validator::validate(Context *c, const ParamsMultiMap &params, ValidatorFlags flags) const
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
        c->setStash(QStringLiteral("validationErrorStrings"), result.errorStrings());
        c->setStash(QStringLiteral("validationErrors"), QVariant::fromValue(result.errors()));

        if (!params.isEmpty()) {
            auto i = params.constBegin();
            while (i != params.constEnd()) {
                if (!i.key().contains(QStringLiteral("password"), Qt::CaseInsensitive)) {
                    c->setStash(i.key(), i.value());
                }
                ++i;
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
    app->loadTranslations(QStringLiteral("plugin_utils_validator"));
}
