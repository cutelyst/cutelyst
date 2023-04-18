/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorbetween_p.h"

using namespace Cutelyst;

ValidatorBetween::ValidatorBetween(const QString &field, QMetaType::Type type, const QVariant &min, const QVariant &max, const ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorBetweenPrivate(field, type, min, max, messages, defValKey))
{
}

ValidatorBetween::~ValidatorBetween()
{
}

ValidatorReturnType ValidatorBetween::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    Q_D(const ValidatorBetween);

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
                qCWarning(C_VALIDATOR, "ValidatorBetween: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qlonglong min = d->extractLongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, -1);
                    qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    const qlonglong max = d->extractLongLong(c, params, d->max, &ok);
                    if (Q_UNLIKELY(!ok)) {
                        result.errorMessage = validationDataError(c, 1);
                        qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                    } else {
                        if ((val < min) || (val > max)) {
                            result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}, {QStringLiteral("max"), max}});
                            qCDebug(C_VALIDATOR, "ValidatorBetween: Validation failed for field %s in %s::%s: %lli is not between %lli and %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min, max);
                        } else {
                            valid = true;
                        }
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
                qCWarning(C_VALIDATOR, "ValidatorBetween: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qulonglong min = d->extractULongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, -1);
                    qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    const qulonglong max = d->extractULongLong(c, params, d->max, &ok);
                    if (Q_UNLIKELY(!ok)) {
                        result.errorMessage = validationDataError(c, 1);
                        qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                    } else {
                        if ((val < min) || (val > max)) {
                            result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}, {QStringLiteral("max"), max}});
                            qCDebug(C_VALIDATOR, "ValidatorBetween: Validation failed for field %s in %s::%s: %llu is not between %llu and %llu.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min, max);
                        } else {
                            valid = true;
                        }
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
                qCWarning(C_VALIDATOR, "ValidatorBetween: Failed to parse value of field %s into number at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const double min = d->extractDouble(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, -1);
                    qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    const double max = d->extractDouble(c, params, d->max, &ok);
                    if (Q_UNLIKELY(!ok)) {
                        result.errorMessage = validationDataError(c, 1);
                        qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                    } else {
                        if ((val < min) || (val > max)) {
                            result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}, {QStringLiteral("max"), max}});
                            qCDebug(C_VALIDATOR, "ValidatorBetween: Validation failed for field %s in %s::%s: %f is not between %f and %f.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min, max);
                        } else {
                            valid = true;
                        }
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
                qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid minimum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
            } else {
                const qlonglong max = d->extractLongLong(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR, "ValidatorBetween: Invalid maximum comparison value for field %s in %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                } else {
                    if ((val < min) || (val > max)) {
                        result.errorMessage = validationError(c, QVariantMap{{QStringLiteral("val"), val}, {QStringLiteral("min"), min}, {QStringLiteral("max"), max}});
                        qCDebug(C_VALIDATOR, "ValidatorBetween: Validation failed for field %s in %s::%s: string length %lli is not between %lli and %lli.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), val, min, max);
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        default:
            qCWarning(C_VALIDATOR, "ValidatorBetween: The comparison type with ID %i for field %s at %s::%s is not supported.", static_cast<int>(d->type), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
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
        defaultValue(c, &result, "ValidatorBetween");
    }

    return result;
}

QString ValidatorBetween::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorBetween);

    const QVariantMap map = errorData.toMap();
    QString min, max;
    switch (d->type) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        min = c->locale().toString(map.value(QStringLiteral("min")).toLongLong());
        max = c->locale().toString(map.value(QStringLiteral("max")).toLongLong());
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        min = c->locale().toString(map.value(QStringLiteral("min")).toULongLong());
        max = c->locale().toString(map.value(QStringLiteral("max")).toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        min = c->locale().toString(map.value(QStringLiteral("min")).toDouble());
        max = c->locale().toString(map.value(QStringLiteral("max")).toDouble());
        break;
    default:
        error = validationDataError(c);
        return error;
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            error = c->translate("Cutelyst::ValidatorBetween", "The text must be between %1 and %2 characters long.").arg(min, max);
        } else {
            error = c->translate("Cutelyst::ValidatorBetween", "The value must be between %1 and %2.").arg(min, max);
        }
    } else {
        if (d->type == QMetaType::QString) {
            error = c->translate("Cutelyst::ValidatorBetween", "The text in the “%1“ field must be between %2 and %3 characters long.").arg(_label, min, max);
        } else {
            error = c->translate("Cutelyst::ValidatorBetween", "The value in the “%1” field must be between %2 and %3.").arg(_label, min, max);
        }
    }

    return error;
}

QString ValidatorBetween::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;

    int field            = errorData.toInt();
    const QString _label = label(c);

    if (field == -1) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorBetween", "The minimum comparison value is not valid.");
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorBetween", "The minimum comparison value for the “%1” field is not valid.").arg(_label);
        }
    } else if (field == 0) {
        Q_D(const ValidatorBetween);
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorBetween", "The comparison type with ID %1 is not supported.").arg(static_cast<int>(d->type));
        } else {
            //: %1 will be replaced by the type id, %2 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorBetween", "The comparison type with ID %1 for the “%2” field is not supported.").arg(QString::number(static_cast<int>(d->type)), _label);
        }
    } else if (field == 1) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorBetween", "The maximum comparison value is not valid.");
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorBetween", "The maximum comparison value for the “%1” field is not valid.").arg(_label);
        }
    }

    return error;
}

QString ValidatorBetween::genericParsingError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    Q_D(const ValidatorBetween);

    const QString _label = label(c);
    if ((d->type == QMetaType::Float) || (d->type == QMetaType::Double)) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorBetween", "Failed to parse the input value into a floating point number.");
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorBetween", "Failed to parse the input value for the “%1” field into a floating point number.").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorBetween", "Failed to parse the input value into an integer number.");
        } else {
            //: %1 will be replaced by the field label
            error = c->translate("Cutelyst::ValidatorBetween", "Failed to parse the input value for the “%1” field into an integer number.").arg(_label);
        }
    }

    return error;
}
