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
    QString v;

    Q_D(const ValidatorRule);

    if (!field().isEmpty() && !d->parameters.isEmpty()) {
        if (trimBefore()) {
            v = d->parameters.value(field()).trimmed();
        } else {
            v = d->parameters.value(field());
        }
    }

    return v;
}

ParamsMultiMap ValidatorRule::parameters() const { Q_D(const ValidatorRule); return d->parameters; }

void ValidatorRule::setParameters(const ParamsMultiMap &params)
{
    Q_D(ValidatorRule);
    d->parameters = params;
}

QString ValidatorRule::fieldLabel() const
{
    return !label().isEmpty() ? label() : field();
}

QString ValidatorRule::validationError() const
{
    QString error;
    Q_D(const ValidatorRule);
    if (d->customError.isEmpty()) {
        error = genericValidationError();
    } else {
        error = d->customError;
    }
    return error;
}

QString ValidatorRule::genericValidationError() const
{
    return QStringLiteral("The input data in the “%1” field is not valid.").arg(fieldLabel());
}

QString ValidatorRule::parsingError() const
{
    QString error;
    Q_D(const ValidatorRule);
    if (d->customParsingError.isEmpty()) {
        error = genericParsingError();
    } else {
        error = d->customParsingError;
    }
    return error;
}

QString ValidatorRule::genericParsingError() const
{
    return QStringLiteral("Failed to parse the input data of the “%1” field.").arg(fieldLabel());
}

QString ValidatorRule::validationDataError() const
{
    QString error;
    Q_D(const ValidatorRule);
    if (d->customValidationDataError.isEmpty()) {
        error = genericValidationDataError();
    } else {
        error = d->customValidationDataError;
    }
    return error;
}

QString ValidatorRule::genericValidationDataError() const
{
    return QStringLiteral("Missing or unusable validation data for the “%1” field.").arg(fieldLabel());
}

void ValidatorRule::setCustomError(const QString &customError)
{
    Q_D(ValidatorRule);
    d->customError = customError;
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

bool ValidatorRule::trimBefore() const { Q_D(const ValidatorRule); return d->trimBefore; }

void ValidatorRule::setTrimBefore(bool trimBefore)
{
    Q_D(ValidatorRule);
    d->trimBefore = trimBefore;
}
