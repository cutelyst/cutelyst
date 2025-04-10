/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorjson_p.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

using namespace Cutelyst;

ValidatorJson::ValidatorJson(const QString &field,
                             ExpectedType expectedType,
                             const Cutelyst::ValidatorMessages &messages,
                             const QString &defValKey)
    : ValidatorRule(*new ValidatorJsonPrivate(field, expectedType, messages, defValKey))
{
}

ValidatorJson::~ValidatorJson() = default;

ValidatorReturnType ValidatorJson::validate(Cutelyst::Context *c,
                                            const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorJson);

    const QString v = value(params);

    if (!v.isEmpty()) {
        QJsonParseError jpe;
        const QJsonDocument json = QJsonDocument::fromJson(v.toUtf8(), &jpe);
        if (jpe.error == QJsonParseError::NoError) {
            if (d->expectedType == ExpectedType::Array && !json.isArray()) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote()
                    << debugString(c) << "A JSON array is expected but not provided";
            } else if (d->expectedType == ExpectedType::Object && !json.isObject()) {
                result.errorMessage = validationError(c);
                qCDebug(C_VALIDATOR).noquote()
                    << debugString(c) << "A JSON object is expected but not provided";
            } else {
                switch (d->expectedType) {
                    using enum ValidatorJson::ExpectedType;
                case Array:
                    result.value.setValue(json.array());
                    break;
                case Object:
                    result.value.setValue(json.object());
                    break;
                case All:
                    result.value.setValue(json);
                    break;
                }
            }
        } else {
            result.errorMessage = validationError(c, jpe.errorString());
            qCDebug(C_VALIDATOR).noquote() << debugString(c) << jpe.errorString();
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

void ValidatorJson::validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const
{
    cb(validate(c, params));
}

QString ValidatorJson::genericValidationError(Context *c, const QVariant &errorData) const
{
    const QString _label = label(c);
    if (errorData.isNull()) {
        Q_D(const ValidatorJson);
        if (d->expectedType == ExpectedType::Array) {
            if (_label.isEmpty()) {
                //% "Not a JSON array."
                return c->qtTrId("cutelyst-valjson-genvalerr-exparray");
            } else {
                //: %1 will be replaced by the field label
                //% "The data entered in the “%1” field is not a JSON array."
                return c->qtTrId("cutelyst-valjson-genvalerr-exparray-label");
            }
        } else {
            if (_label.isEmpty()) {
                //% "Not a JSON object."
                return c->qtTrId("cutelyst-valjson-genvalerr-expobject");
            } else {
                //: %1 will be replaced by the field label
                //% "The data entered in the “%1” field is not a JSON object."
                return c->qtTrId("cutelyst-valjson-genvalerr-expobject-label");
            }
        }
    } else {
        const QString jsonError = errorData.toString();
        if (_label.isEmpty()) {
            if (!jsonError.isEmpty()) {
                //: %1 will contain the json error
                //% "Invalid JSON data: %1"
                return c->qtTrId("cutelyst-valjson-genvalerr-data").arg(jsonError);
            } else {
                //% "Invalid JSON data."
                return c->qtTrId("cutelyst-valjson-genvalerr");
            }
        } else {
            if (!jsonError.isEmpty()) {
                //: %1 will contain the field label, %2 will contain the json error
                //% "The data entered in the “%1” field is not valid JSON: %2"
                return c->qtTrId("cutelyst-valjson-genvalerr-data-label").arg(_label, jsonError);
            } else {
                //: %1 will be replaced by the field label
                //% "The data entered in the “%1” field is not valid JSON."
                return c->qtTrId("cutelyst-valjson-genvalerr-label").arg(_label);
            }
        }
    }
}
