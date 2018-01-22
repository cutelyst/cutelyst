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

#include "validatoraccepted_p.h"
#include <QStringList>

using namespace Cutelyst;

ValidatorAccepted::ValidatorAccepted(const QString &field, const Cutelyst::ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorAcceptedPrivate(field, messages))
{

}

ValidatorAccepted::~ValidatorAccepted()
{

}

ValidatorReturnType ValidatorAccepted::validate(Cutelyst::Context *c, const Cutelyst::ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (Q_LIKELY(ValidatorAccepted::validate(value(params)))) {
        result.value.setValue<bool>(true);
    } else {
        result.errorMessage = validationError(c);
        result.value.setValue<bool>(false);
        qCDebug(C_VALIDATOR, "ValidatorAccepted: Validation failed for field %s at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
    }

    return result;
}

bool ValidatorAccepted::validate(const QString &value)
{
    bool ret = true;
    static const QStringList l({QStringLiteral("yes"), QStringLiteral("on"), QStringLiteral("1"), QStringLiteral("true")});
    ret = l.contains(value, Qt::CaseInsensitive);
    return ret;
}

QString ValidatorAccepted::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorAccepted", "Has to be accepted.");
    } else {
        error = c->translate("Cutelyst::Validator", "“%1” has to be accepted.");
    }
    return error;
}
