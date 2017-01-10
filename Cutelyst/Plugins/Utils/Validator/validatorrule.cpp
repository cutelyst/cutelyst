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

#include "validatorrule_p.h"

using namespace Cutelyst;

ValidatorRule::ValidatorRule(const QString &field, const QString &label, const QString &customError) :
    d_ptr(new ValidatorRulePrivate(field, label, customError))
{
}

ValidatorRule::ValidatorRule(ValidatorRulePrivate &dd) :
    d_ptr(&dd)
{
}

ValidatorRule::~ValidatorRule()
{
}

QString ValidatorRule::errorMessage() const
{
    Q_D(const ValidatorRule);
    if (d->parsingError) {
        return parsingErrorMessage();
    } else if (d->validationDataError) {
        return validationDataErrorMessage();
    } else if (!d->customError.isEmpty()) {
        return d->customError;
    } else {
        return genericErrorMessage();
    }
}

QString ValidatorRule::field() const { Q_D(const ValidatorRule); return d->field; }

void ValidatorRule::setField(const QString &field)
{
    Q_D(ValidatorRule);
    d->field = field;
}

QString ValidatorRule::label() const { Q_D(const ValidatorRule); return d->label; }

void ValidatorRule::setLabel(const QString &label)
{
    Q_D(ValidatorRule);
    d->label = label;
}

QString ValidatorRule::value() const
{
    Q_D(const ValidatorRule);
    if (!field().isEmpty() && !d->parameters.isEmpty()) {
        if (trimBefore()) {
            return d->parameters.value(field()).trimmed();
        } else {
            return d->parameters.value(field());
        }
    } else {
        return QString();
    }
}

QString ValidatorRule::customError() const { Q_D(const ValidatorRule); return d->customError; }

void ValidatorRule::setCustomError(const QString &customError)
{
    Q_D(ValidatorRule);
    d->customError = customError;
}

bool ValidatorRule::isValid() const { Q_D(const ValidatorRule); return d->valid; }

void ValidatorRule::setValid(bool valid)
{
    Q_D(ValidatorRule);
    d->valid = valid;
}

void ValidatorRule::setParameters(const ParamsMultiMap &params)
{
    Q_D(ValidatorRule);
    d->parameters = params;
}

ParamsMultiMap ValidatorRule::parameters() const
{
    Q_D(const ValidatorRule);
    return d->parameters;
}

QString ValidatorRule::genericFieldName() const
{
    return !label().isEmpty() ? label() : field();
}

void ValidatorRule::setParsingError(bool parsingError)
{
    Q_D(ValidatorRule);
    d->parsingError = parsingError;
    if (parsingError) {
        d->valid = false;
    }
}

bool ValidatorRule::parsingError() const
{
    Q_D(const ValidatorRule);
    return d->parsingError;
}

void ValidatorRule::setValidationDataError(bool validationDataError)
{
    Q_D(ValidatorRule);
    d->validationDataError = validationDataError;
    if (validationDataError) {
        d->valid = false;
    }
}

bool ValidatorRule::validationDataError() const
{
    Q_D(const ValidatorRule);
    return d->validationDataError;
}

QString ValidatorRule::genericErrorMessage() const
{
    return QStringLiteral("The input data in the “%1” field is not valid.").arg(genericFieldName());
}

QString ValidatorRule::parsingErrorMessage() const
{
    Q_D(const ValidatorRule);
    if (d->customParsingError.isEmpty()) {
        return QStringLiteral("Failed to parse the input data of the “%1” field.").arg(genericFieldName());
    } else {
        return d->customParsingError;
    }
}

QString ValidatorRule::validationDataErrorMessage() const
{
    Q_D(const ValidatorRule);
    if (d->customValidationDataError.isEmpty()) {
        return QStringLiteral("Missing or unusable validation data for the “%1” field.").arg(genericFieldName());
    } else {
        return d->customValidationDataError;
    }
}

void ValidatorRule::setCustomParsingError(const QString &custom)
{
    Q_D(ValidatorRule);
    d->customParsingError = custom;
}

void ValidatorRule::setCustomValidationDataError(const QString &custom)
{
    Q_D(ValidatorRule);
    d->customValidationDataError = custom;
}

void ValidatorRule::setTrimBefore(bool trimBefore)
{
    Q_D(ValidatorRule);
    d->trimBefore = trimBefore;
}

bool ValidatorRule::trimBefore() const
{
    Q_D(const ValidatorRule);
    return d->trimBefore;
}
