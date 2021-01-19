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

#include "validatorregularexpression_p.h"

using namespace Cutelyst;

ValidatorRegularExpression::ValidatorRegularExpression(const QString &field, const QRegularExpression &regex, const ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorRegularExpressionPrivate(field, regex, messages, defValKey))
{
}

ValidatorRegularExpression::~ValidatorRegularExpression()
{
}

ValidatorReturnType ValidatorRegularExpression::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorRegularExpression);

    const QString v = value(params);

    if (d->regex.isValid()) {
        if (!v.isEmpty()) {
            if (v.contains(d->regex)) {
                result.value.setValue(v);
            } else {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR, "ValidatorRegularExpression: Validation failed for field %s at %s::%s because value does not match the following regular expression: %s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(d->regex.pattern()));
            }
        } else {
            defaultValue(c, &result, "ValidatorRegularExpression");
        }
    } else {
        result.errorMessage = validationDataError(c);
        qCWarning(C_VALIDATOR, "ValidatorRegularExpression: the regular expression for the field %s at %s::%s is not valid: %s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(d->regex.errorString()));
    }

    return result;
}

QString ValidatorRegularExpression::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorRegularExpression", "Does not match the desired format.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorRegularExpression", "The “%1” field does not match the desired format.").arg(_label);
    }
    return error;
}
