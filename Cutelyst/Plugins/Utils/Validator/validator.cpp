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
#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_VALIDATOR, "cutelyst.utils.validator")

Validator::Validator(Context *c) :
    d_ptr(new ValidatorPrivate(c))
{
}

#ifdef Q_COMPILER_INITIALIZER_LISTS
Validator::Validator(Context *c, std::initializer_list<ValidatorRule *> validators) :
    d_ptr(new ValidatorPrivate(c, validators))
{
}

Validator::Validator(Context *c, std::initializer_list<ValidatorRule *> validators, std::initializer_list<std::pair<QString, QString> > labelDictionary) :
    d_ptr(new ValidatorPrivate(c, validators, labelDictionary))
{
}
#endif

Validator::Validator(const ParamsMultiMap &params) :
    d_ptr(new ValidatorPrivate(params))
{
}

Validator::~Validator()
{
}

void Validator::setStopOnFirstError(bool stopOnFirstError)
{
    Q_D(Validator);
    d->stopOnFirstError = stopOnFirstError;
}

bool Validator::stopOnFirstError() const
{
    Q_D(const Validator);
    return d->stopOnFirstError;
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

bool Validator::validate()
{
    Q_D(Validator);

    if (d->validators.empty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty validator list.";
        return true;
    }

    if (d->params.isEmpty()) {
        qCWarning(C_VALIDATOR) << "Validation started with empty parameters.";
    }

    bool valid = true;

    for (std::vector<ValidatorRule*>::iterator it = d->validators.begin(); it != d->validators.end(); ++it) {
        ValidatorRule *v = *it;
        v->setParameters(d->params);

        if (v->label().isEmpty()) {
            v->setLabel(d->labelDict.value(v->field()));
        }

        if (!v->validate()) {
            if (stopOnFirstError()) {
                d->setStashOnInvalid();
                return false;
            } else {
                valid = false;
            }
        }
    }

    if (!valid) {
        d->setStashOnInvalid();
    }

    return valid;
}

void Validator::addValidator(ValidatorRule *v)
{
    Q_D(Validator);
    d->validators.push_back(v);
}

QVariantList Validator::errorStrings() const
{
    Q_D(const Validator);

    QVariantList strings;

    if (!d->validators.empty()) {
        for (std::vector<ValidatorRule*>::const_iterator it = d->validators.cbegin(); it != d->validators.cend(); ++it) {
            ValidatorRule *v = *it;
            if (!v->isValid()) {
                strings << v->errorMessage();
            }
        }
    }

    return strings;
}

QVariantList Validator::errorFields() const
{
    Q_D(const Validator);

    QVariantList fields;

    if (!d->validators.empty()) {
        for (std::vector<ValidatorRule*>::const_iterator it = d->validators.cbegin(); it != d->validators.cend(); ++it) {
            ValidatorRule *v = *it;
            if (!v->isValid()) {
                fields << v->field();
            }
        }
    }

    return fields;
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

void Validator::setTemplate(const QString &tmpl)
{
    Q_D(Validator);
    d->tmpl = tmpl;
}
