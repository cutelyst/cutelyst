/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "validator_p.h"
#include <Cutelyst/context.h>
#include <Cutelyst/request.h>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_VALIDATOR, "cutelyst.utils.validator")

Validator::Validator(const QLatin1String &translationContext) :
    d_ptr(new ValidatorPrivate(translationContext))
{
}

#ifdef Q_COMPILER_INITIALIZER_LISTS
Validator::Validator(std::initializer_list<ValidatorRule *> validators, const QLatin1String &translationContext) :
    d_ptr(new ValidatorPrivate(validators, translationContext))
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
        params = c->req()->parameters();
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
    const bool noTrimming = flags.testFlag(NoTrimming);

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
            QMap<QString,QString>::const_iterator i = params.constBegin();
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
