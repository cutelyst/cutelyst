/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatormin_p.h"

using namespace Cutelyst;

ValidatorMin::ValidatorMin(const QString &field, QMetaType::Type type, const QVariant &min, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorMinPrivate(field, type, min, messages, defValKey))
{
}

ValidatorMin::~ValidatorMin()
{
}

ValidatorReturnType ValidatorMin::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    Q_D(const ValidatorMin);

    if (!v.isEmpty()) {
        bool ok    = false;
        bool valid = false;

        switch (d->type) {
        case QMetaType::Char:
        case QMetaType::Short:
        case QMetaType::Int:
        case QMetaType::Long:
        case QMetaType::LongLong:
        {
            const qlonglong val = c->locale().toLongLong(v, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR, "ValidatorMin: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qlonglong min = d->extractLongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, -1);
                    qCWarning(C_VALIDATOR, "ValidatorMin: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val < min) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}});
                        qCDebug(C_VALIDATOR, "ValidatorMin: Validation failed for field %s in %s::%s: %lli is not greater than %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min);
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::UChar:
        case QMetaType::UShort:
        case QMetaType::UInt:
        case QMetaType::ULong:
        case QMetaType::ULongLong:
        {
            const qulonglong val = v.toULongLong(&ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR, "ValidatorMin: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qulonglong min = d->extractULongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, -1);
                    qCWarning(C_VALIDATOR, "ValidatorMin: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val < min) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}});
                        qCDebug(C_VALIDATOR, "ValidatorMin: Validation failed for field %s in %s::%s: %llu is not greater than %llu.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min);
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
                qCWarning(C_VALIDATOR, "ValidatorMin: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const double min = d->extractDouble(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, -1);
                    qCWarning(C_VALIDATOR, "ValidatorMin: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val < min) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}});
                        qCDebug(C_VALIDATOR, "ValidatorMin: Validation failed for field %s in %s::%s: %f is not greater than %f.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min);
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::QString:
        {
            const qlonglong val = static_cast<qlonglong>(v.length());
            const qlonglong min = d->extractLongLong(c, params, d->min, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = validationDataError(c, -1);
                qCWarning(C_VALIDATOR, "ValidatorMin: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                if (val < min) {
                    result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}});
                    qCDebug(C_VALIDATOR, "ValidatorMin: Validation failed for field %s in %s::%s: string length %lli is not longer than %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min);
                } else {
                    valid = true;
                }
            }
        } break;
        default:
            qCWarning(C_VALIDATOR, "ValidatorMin: The comparison type with ID %i for field %s at %s::%s is not supported.", static_cast<int>(d->type), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
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
        defaultValue(c, &result, "ValidatorMin");
    }

    return result;
}

QString ValidatorMin::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorMin);

    const QVariantMap map = errorData.toMap();
    QString min;
    switch (d->type) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        min = c->locale().toString(map.value(QStringLiteral("min")).toLongLong());
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        min = c->locale().toString(map.value(QStringLiteral("min")).toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        min = c->locale().toString(map.value(QStringLiteral("min")).toDouble());
        break;
    default:
        error = validationDataError(c);
        return error;
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            error = c->translate("Cutelyst::ValidatorMin", "The text must be longer than %1 characters.").arg(min);
        } else {
            error = c->translate("Cutelyst::ValidatorMin", "The value must be greater than %1.").arg(min);
        }
    } else {
        if (d->type == QMetaType::QString) {
            error = c->translate("Cutelyst::ValidatorMin", "The text in the “%1“ field must be longer than %2 characters.").arg(_label, min);
        } else {
            error = c->translate("Cutelyst::ValidatorMin", "The value in the “%1” field must be greater than %2.").arg(_label, min);
        }
    }

    return error;
}

QString ValidatorMin::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;

    int field            = errorData.toInt();
    const QString _label = label(c);

    if (field == -1) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMin", "The minimum comparison value is not valid.");
        } else {
            error = c->translate("Cutelyst::ValidatorMin", "The minimum comparison value for the “%1” field is not valid.").arg(_label);
        }
    } else if (field == 0) {
        Q_D(const ValidatorMin);
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMin", "The comparison type with ID %1 is not supported.").arg(static_cast<int>(d->type));
        } else {
            error = c->translate("Cutelyst::ValidatorMin", "The comparison type with ID %1 for the “%2” field is not supported.").arg(QString::number(static_cast<int>(d->type)), _label);
        }
    }

    return error;
}

QString ValidatorMin::genericParsingError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorMin);

    const QString _label = label(c);
    if ((d->type == QMetaType::Float) || (d->type == QMetaType::Double)) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMin", "Failed to parse the input value into a floating point number.");
        } else {
            error = c->translate("Cutelyst::ValidatorMin", "Failed to parse the input value for the “%1” field into a floating point number.").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMin", "Failed to parse the input value into an integer number.");
        } else {
            error = c->translate("Cutelyst::ValidatorMin", "Failed to parse the input value for the “%1” field into an integer number.").arg(_label);
        }
    }

    return error;
}
