/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorsize_p.h"

using namespace Cutelyst;

ValidatorSize::ValidatorSize(const QString &field, QMetaType::Type type, const QVariant &size, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorSizePrivate(field, type, size, messages, defValKey))
{
}

ValidatorSize::~ValidatorSize()
{
}

ValidatorReturnType ValidatorSize::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {

        Q_D(const ValidatorSize);
        bool ok    = false;
        bool valid = false;

        switch (d->type) {
        case QMetaType::Short:
        case QMetaType::Int:
        case QMetaType::Long:
        case QMetaType::LongLong:
        {
            const qlonglong val = c->locale().toLongLong(v, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR, "ValidatorSize: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qlonglong size = d->extractLongLong(c, params, d->size, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR, "ValidatorSize: Invalid comparison size for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val != size) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("size"), size}});
                        qCDebug(C_VALIDATOR, "ValidatorSize: Validation failed for field %s in %s::%s: value is not %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), size);
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::UShort:
        case QMetaType::UInt:
        case QMetaType::ULong:
        case QMetaType::ULongLong:
        {
            const qulonglong val = v.toULongLong(&ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR, "ValidatorSize: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qulonglong size = d->extractULongLong(c, params, d->size, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR, "ValidatorSize: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val != size) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("size"), size}});
                        qCDebug(C_VALIDATOR, "ValidatorSize: Validation failed for field %s in %s::%s: value is not %llu.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), size);
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::Float:
        case QMetaType::Double:
        {
            const double val = v.toDouble(&ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR, "ValidatorSize: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const double size = d->extractDouble(c, params, d->size, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR, "ValidatorSize: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val != size) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("size"), size}});
                        qCDebug(C_VALIDATOR, "ValidatorSize: Validation failed for field %s in %s::%s: value is not %f.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), size);
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::QString:
        {
            const qlonglong val  = static_cast<qlonglong>(v.length());
            const qlonglong size = d->extractLongLong(c, params, d->size, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = validationDataError(c, 1);
                qCWarning(C_VALIDATOR, "ValidatorSize: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                if (val != size) {
                    result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("size"), size}});
                    qCDebug(C_VALIDATOR, "ValidatorSize: Validation failed for field %s in %s::%s: string length is not %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), size);
                } else {
                    valid = true;
                }
            }
        } break;
        default:
            qCWarning(C_VALIDATOR, "ValidatorSize: The comparison type with ID %i for field %s at %s::%s is not supported.", static_cast<int>(d->type), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            result.errorMessage = validationDataError(c, 0);
            break;
        }

        if (valid) {
            if (d->type != QMetaType::QString) {
                const QVariant _v = d->valueToNumber(c, v, d->type);
                if (_v.isValid()) {
                    result.value = _v;
                } else {
                    result.errorMessage = parsingError(c);
                }
            } else {
                result.value.setValue(v);
            }
        }
    } else {
        defaultValue(c, &result, "ValidatorSize");
    }

    return result;
}

QString ValidatorSize::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorSize);

    const QVariantMap map = errorData.toMap();
    QString size;
    switch (d->type) {
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        size = c->locale().toString(map.value(QStringLiteral("size")).toLongLong());
        break;
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        size = c->locale().toString(map.value(QStringLiteral("size")).toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        size = c->locale().toString(map.value(QStringLiteral("size")).toDouble());
        break;
    default:
        error = validationDataError(c);
        return error;
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            //: %1 will be replaced by the required string size
            error = c->translate("Cutelyst::ValidatorSize", "The text must be exactly %1 characters long.").arg(size);
        } else {
            //: %1 will be replaced by the required size/value
            error = c->translate("Cutelyst::ValidatorSize", "The value must be %1.").arg(size);
        }
    } else {
        if (d->type == QMetaType::QString) {
            //: %1 will be replaced by the field label, %2 will be replaced by the required string size
            error = c->translate("Cutelyst::ValidatorSize", "The text in the “%1“ field must be exactly %2 characters long.").arg(_label, size);
        } else {
            //: %1 will be replaced by the field label, %2 will be replaced by the required size/value
            error = c->translate("Cutelyst::ValidatorSize", "The value in the “%1” field must be %2.").arg(_label, size);
        }
    }

    return error;
}

QString ValidatorSize::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;

    int field            = errorData.toInt();
    const QString _label = label(c);

    if (field == 0) {
        Q_D(const ValidatorSize);
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorSize", "The comparison type with ID %1 is not supported.").arg(static_cast<int>(d->type));
        } else {
            error = c->translate("Cutelyst::ValidatorSize", "The comparison type with ID %1 for the “%2” field is not supported.").arg(QString::number(static_cast<int>(d->type)), _label);
        }
    } else if (field == 1) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorSize", "The comparison value is not valid.");
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorSize", "The comparison value for the “%1” field is not valid.").arg(_label);
        }
    }

    return error;
}

QString ValidatorSize::genericParsingError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorSize);

    const QString _label = label(c);
    if ((d->type == QMetaType::Float) || (d->type == QMetaType::Double)) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorSize", "Failed to parse the input value into a floating point number.");
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorSize", "Failed to parse the input value for the “%1” field into a floating point number.").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorSize", "Failed to parse the input value into an integer number.");
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorSize", "Failed to parse the input value for the “%1” field into an integer number.").arg(_label);
        }
    }

    return error;
}
