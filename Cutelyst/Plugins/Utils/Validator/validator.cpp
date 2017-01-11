/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "validator_p.h"
#include <Cutelyst/context.h>
#include <Cutelyst/request.h>
#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_VALIDATOR, "cutelyst.utils.validator")

Validator::Validator() :
    d_ptr(new ValidatorPrivate)
{
}

#ifdef Q_COMPILER_INITIALIZER_LISTS
Validator::Validator(std::initializer_list<ValidatorRule *> validators) :
    d_ptr(new ValidatorPrivate(validators))
{
}

Validator::Validator(std::initializer_list<ValidatorRule *> validators, std::initializer_list<std::pair<QString, QString> > labelDictionary) :
    d_ptr(new ValidatorPrivate(validators, labelDictionary))
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
    if (!c) {
        qCWarning(C_VALIDATOR) << "No valid Context set, aborting validation.";
        return ValidatorResult();
    }

    const ParamsMultiMap params = c->request()->parameters();
    const ValidatorResult result = validate(params, flags);

    if (!result && flags.testFlag(FillStashOnError)) {
        c->setStash(QStringLiteral("validationErrorStrings"), result.errorStrings());
        c->setStash(QStringLiteral("validationErrorFields"), result.errorFields());

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

ValidatorResult Validator::validate(const ParamsMultiMap &params, ValidatorFlags flags) const
{
    Q_D(const Validator);

    ValidatorResult result;

    if (d->validators.empty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty validator list.";
        return result;
    }

    if (params.isEmpty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty parameters.";
    }

    const bool stopOnFirstError = flags.testFlag(StopOnFirstError);
    const bool noTrimming = flags.testFlag(NoTrimming);

    for (auto it = d->validators.cbegin(); it != d->validators.cend(); ++it) {
        ValidatorRule *v = *it;
        v->setParameters(params);

        if (v->label().isEmpty()) {
            v->setLabel(d->labelDict.value(v->field()));
        }

        if (noTrimming) {
            v->setTrimBefore(false);
        }

        const QString singleResult = v->validate();

        if (!singleResult.isEmpty()) {
            result.addError(v->field(), singleResult);

            if (stopOnFirstError) {
                return result;
            }
        }
    }

    return result;
}

void Validator::addValidator(ValidatorRule *v)
{
    Q_D(Validator);
    d->validators.push_back(v);
}

void Validator::setLabelDictionary(const QHash<QString, QString> &labelDict)
{
    Q_D(Validator);
    d->labelDict = labelDict;
}

void Validator::addLabelDictionary(const QHash<QString, QString> &labelDict)
{
    Q_D(Validator);
    if (!labelDict.isEmpty()) {
        QHash<QString, QString>::const_iterator i = labelDict.constBegin();
        while (i != labelDict.constEnd()) {
            d->labelDict.insert(i.key(), i.value());
        }
    }
}

void Validator::addLabel(const QString &field, const QString &label)
{
    Q_D(Validator);
    d->labelDict.insert(field, label);
}
