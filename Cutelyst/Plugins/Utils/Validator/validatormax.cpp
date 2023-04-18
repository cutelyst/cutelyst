/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatormax_p.h"

using namespace Cutelyst;

ValidatorMax::ValidatorMax(const QString &field, QMetaType::Type type, const QVariant &max, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorMaxPrivate(field, type, max, messages, defValKey))
{
}

ValidatorMax::~ValidatorMax()
{
}

ValidatorReturnType ValidatorMax::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        Q_D(const ValidatorMax);
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
                qCWarning(C_VALIDATOR, "ValidatorMax: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qlonglong max = d->extractLongLong(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR, "ValidatorMax: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val > max) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("max"), max}});
                        qCDebug(C_VALIDATOR, "ValidatorMax: Validation failed for field %s in %s::%s: %lli is not smaller than %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, max);
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
                qCWarning(C_VALIDATOR, "ValidatorMax: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qulonglong max = d->extractULongLong(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR, "ValidatorMax: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val > max) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("max"), max}});
                        qCDebug(C_VALIDATOR, "ValidatorMax: Validation failed for field %s in %s::%s: %llu is not smaller than %llu.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, max);
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
                qCWarning(C_VALIDATOR, "ValidatorMax: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const double max = d->extractDouble(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR, "ValidatorMax: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if (val > max) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("max"), max}});
                        qCDebug(C_VALIDATOR, "ValidatorMax: Validation failed for field %s in %s::%s: %f is not smaller than %f.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, max);
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::QString:
        {
            const qlonglong val = static_cast<qlonglong>(v.length());
            const qlonglong max = d->extractLongLong(c, params, d->max, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = validationDataError(c, 1);
                qCWarning(C_VALIDATOR, "ValidatorMax: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                if (val > max) {
                    result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("max"), max}});
                    qCDebug(C_VALIDATOR, "ValidatorMax: Validation failed for field %s in %s::%s: string length %lli is not smaller than %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, max);
                } else {
                    valid = true;
                }
            }
        } break;
        default:
            qCWarning(C_VALIDATOR, "ValidatorMax: The comparison type with ID %i for field %s at %s::%s is not supported.", static_cast<int>(d->type), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
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
        defaultValue(c, &result, "ValidatorMax");
    }

    return result;
}

QString ValidatorMax::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorMax);

    const QVariantMap map = errorData.toMap();
    QString max;
    switch (d->type) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        max = c->locale().toString(map.value(QStringLiteral("max")).toLongLong());
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        max = c->locale().toString(map.value(QStringLiteral("max")).toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        max = c->locale().toString(map.value(QStringLiteral("max")).toDouble());
        break;
    default:
        error = validationDataError(c);
        return error;
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            error = c->translate("Cutelyst::ValidatorMax", "The text must be shorter than %1 characters.").arg(max);
        } else {
            error = c->translate("Cutelyst::ValidatorMax", "The value must be lower than %1.").arg(max);
        }
    } else {
        if (d->type == QMetaType::QString) {
            error = c->translate("Cutelyst::ValidatorMax", "The text in the “%1“ field must be shorter than %2 characters.").arg(_label, max);
        } else {
            error = c->translate("Cutelyst::ValidatorMax", "The value in the “%1” field must be lower than %2.").arg(_label, max);
        }
    }

    return error;
}

QString ValidatorMax::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;

    int field            = errorData.toInt();
    const QString _label = label(c);

    if (field == 0) {
        Q_D(const ValidatorMax);
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMax", "The comparison type with ID %1 is not supported.").arg(static_cast<int>(d->type));
        } else {
            error = c->translate("Cutelyst::ValidatorMax", "The comparison type with ID %1 for the “%2” field is not supported.").arg(QString::number(static_cast<int>(d->type)), _label);
        }
    } else if (field == 1) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMax", "The maximum comparison value is not valid.");
        } else {
            error = c->translate("Cutelyst::ValidatorMax", "The maximum comparison value for the “%1” field is not valid.").arg(_label);
        }
    }

    return error;
}

QString ValidatorMax::genericParsingError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorMax);

    const QString _label = label(c);
    if ((d->type == QMetaType::Float) || (d->type == QMetaType::Double)) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMax", "Failed to parse the input value into a floating point number.");
        } else {
            error = c->translate("Cutelyst::ValidatorMax", "Failed to parse the input value for the “%1” field into a floating point number.").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorMax", "Failed to parse the input value into an integer number.");
        } else {
            error = c->translate("Cutelyst::ValidatorMax", "Failed to parse the input value for the “%1” field into an integer number.").arg(_label);
        }
    }

    return error;
}
