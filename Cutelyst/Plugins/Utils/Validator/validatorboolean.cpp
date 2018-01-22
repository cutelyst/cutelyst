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

#include "validatorboolean_p.h"
#include <QStringList>

using namespace Cutelyst;

ValidatorBoolean::ValidatorBoolean(const QString &field, const ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorBooleanPrivate(field, messages, defValKey))
{
}

ValidatorBoolean::~ValidatorBoolean()
{
}

ValidatorReturnType ValidatorBoolean::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        static const QStringList lt({QStringLiteral("1"), QStringLiteral("true"), QStringLiteral("on")});
        static const QStringList lf({QStringLiteral("0"), QStringLiteral("false"), QStringLiteral("off")});
        if (lt.contains(v, Qt::CaseInsensitive)) {
            result.value.setValue<bool>(true);
        } else if (lf.contains(v, Qt::CaseInsensitive)) {
            result.value.setValue<bool>(false);
        } else {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorBoolean: The value %s of field %s in %s::%s can not be interpreted as boolean.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        }
    } else {
        defaultValue(c, &result, "ValidatorBoolean");
    }

    return result;
}

QString ValidatorBoolean::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData);
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorBoolean", "Can not be interpreted as boolean value.");
    } else {
        error = c->translate("Cutelyst::ValidatorBoolean", "The value in the “%1” field can not be interpreted as a boolean value.").arg(_label);
    }
    return error;
}

