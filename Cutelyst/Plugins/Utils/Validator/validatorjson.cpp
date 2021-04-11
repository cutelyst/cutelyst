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

#include "validatorjson_p.h"
#include <QJsonDocument>
#include <QJsonParseError>

using namespace Cutelyst;

ValidatorJson::ValidatorJson(const QString &field, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorJsonPrivate(field, messages, defValKey))
{
}

ValidatorJson::~ValidatorJson()
{
}

ValidatorReturnType ValidatorJson::validate(Cutelyst::Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        QJsonParseError jpe;
        const QJsonDocument json = QJsonDocument::fromJson(v.toUtf8(), &jpe);
        if (json.isEmpty() || json.isNull()) {
            result.errorMessage = validationError(c, jpe.errorString());
            qCDebug(C_VALIDATOR, "ValidatorJson: Validation failed for field %s at %s::%s with the following error: %s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(jpe.errorString()));
        } else {
            result.value.setValue(json);
        }
    } else {
        defaultValue(c, &result, "ValidatorJson");
    }

    return result;
}

QString ValidatorJson::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    const QString _label = label(c);
    const QString jsonError = errorData.toString();
    if (_label.isEmpty()) {
        if (!jsonError.isEmpty()) {
            //: %1 will contain the json error
            error = c->translate("Cutelyst::ValidatorJson", "Invalid JSON data: %1").arg(jsonError);
        } else {
            error = c->translate("Cutelyst::ValidatorJson", "Invalid JSON data.");
        }
    } else {
        if (!jsonError.isEmpty()) {
            //: %1 will contain the field label, %2 will contain the json error
            error = c->translate("Cutelyst::ValidatorJson", "The data entered in the “%1” field is not valid JSON: %2").arg(_label, jsonError);
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorJson", "The data entered in the “%1” field is not valid JSON.").arg(_label);
        }
    }
    return error;
}
