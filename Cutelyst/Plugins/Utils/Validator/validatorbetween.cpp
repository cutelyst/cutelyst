﻿/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorbetween_p.h"

#include <QMetaType>

using namespace Cutelyst;
using namespace Qt::StringLiterals;

ValidatorBetween::ValidatorBetween(const QString &field,
                                   QMetaType::Type type,
                                   const QVariant &min,
                                   const QVariant &max,
                                   const ValidatorMessages &messages,
                                   const QString &defValKey)
    : ValidatorRule(*new ValidatorBetweenPrivate(field, type, min, max, messages, defValKey))
{
}

ValidatorBetween::~ValidatorBetween() = default;

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
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " Can not parse input \"" << v
                    << "\" into an integer number";
            } else {
                const qlonglong min =
                    ValidatorBetweenPrivate::extractLongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                    qCWarning(C_VALIDATOR).noquote()
                        << "Invalid mininum comparison value:" << d->min;
                } else {
                    const qlonglong max =
                        ValidatorBetweenPrivate::extractLongLong(c, params, d->max, &ok);
                    if (Q_UNLIKELY(!ok)) {
                        result.errorMessage = validationDataError(
                            c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                        qCWarning(C_VALIDATOR).noquote()
                            << "Invalid maximum comparison value:" << d->max;
                    } else {
                        if ((val < min) || (val > max)) {
                            result.errorMessage = validationError(
                                c, QVariantMap{{u"val"_s, val}, {u"min"_s, min}, {u"max"_s, max}});
                            qCDebug(C_VALIDATOR).noquote()
                                << debugString(c) << val << "is not between" << min << "and" << max;
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
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " Can not parse input \"" << v
                    << "\" into an unsigned integer number";
            } else {
                const qulonglong min =
                    ValidatorBetweenPrivate::extractULongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid mininum comparison value:" << d->min;
                } else {
                    const qulonglong max =
                        ValidatorBetweenPrivate::extractULongLong(c, params, d->max, &ok);
                    if (Q_UNLIKELY(!ok)) {
                        result.errorMessage = validationDataError(
                            c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                        qCWarning(C_VALIDATOR).noquote()
                            << debugString(c) << "Invalid maximum comparison value:" << d->max;
                    } else {
                        if ((val < min) || (val > max)) {
                            result.errorMessage = validationError(
                                c, QVariantMap{{u"val"_s, val}, {u"min"_s, min}, {u"max"_s, max}});
                            qCDebug(C_VALIDATOR).noquote()
                                << debugString(c) << val << "is not between" << min << "and" << max;
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
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " Can not parse input \"" << v
                    << "\" into a floating point number";
            } else {
                const double min = ValidatorBetweenPrivate::extractDouble(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid mininum comparison value:" << d->min;
                } else {
                    const double max =
                        ValidatorBetweenPrivate::extractDouble(c, params, d->max, &ok);
                    if (Q_UNLIKELY(!ok)) {
                        result.errorMessage = validationDataError(
                            c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                        qCWarning(C_VALIDATOR).noquote()
                            << debugString(c) << "Invalid maximum comparison value:" << d->max;
                    } else {
                        if ((val < min) || (val > max)) {
                            result.errorMessage = validationError(
                                c, QVariantMap{{u"val"_s, val}, {u"min"_s, min}, {u"max"_s, max}});
                            qCDebug(C_VALIDATOR).noquote()
                                << debugString(c) << val << "is not between" << min << "and" << max;
                        } else {
                            valid = true;
                        }
                    }
                }
            }
        } break;
        case QMetaType::QString:
        {
            const auto val      = static_cast<qlonglong>(v.length());
            const qlonglong min = ValidatorBetweenPrivate::extractLongLong(c, params, d->min, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = validationDataError(
                    c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                qCWarning(C_VALIDATOR).noquote()
                    << debugString(c) << "Invalid mininum comparison value:" << d->min;
            } else {
                const qlonglong max =
                    ValidatorBetweenPrivate::extractLongLong(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid maximum comparison value:" << d->max;
                } else {
                    if ((val < min) || (val > max)) {
                        result.errorMessage = validationError(
                            c, QVariantMap{{u"val"_s, val}, {u"min"_s, min}, {u"max"_s, max}});
                        qCDebug(C_VALIDATOR).noquote() << debugString(c) << "String length" << val
                                                       << "is not between" << min << "and" << max;
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        default:
            qCWarning(C_VALIDATOR).noquote()
                << debugString(c) << "The comparison type" << d->type << "is not supported";
            result.errorMessage = validationDataError(
                c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidType));
            break;
        }

        if (valid) {
            if (d->type != QMetaType::QString) {
                const QVariant _v = ValidatorBetweenPrivate::valueToNumber(c, v, d->type);
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
        defaultValue(c, &result);
    }

    return result;
}

void ValidatorBetween::validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const
{
    cb(validate(c, params));
}

QString ValidatorBetween::genericValidationError(Cutelyst::Context *c,
                                                 const QVariant &errorData) const
{
    Q_D(const ValidatorBetween);

    const QVariantMap map = errorData.toMap();
    QString min;
    QString max;
    switch (d->type) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        min = c->locale().toString(map.value(u"min"_s).toLongLong());
        max = c->locale().toString(map.value(u"max"_s).toLongLong());
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        min = c->locale().toString(map.value(u"min"_s).toULongLong());
        max = c->locale().toString(map.value(u"max"_s).toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        min = c->locale().toString(map.value(u"min"_s).toDouble());
        max = c->locale().toString(map.value(u"max"_s).toDouble());
        break;
    default:
        return validationDataError(c);
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            //: %1 will be replaced by the minimum, %2 by the maximum value
            //% "The text must be between %1 and %2 characters long."
            return c->qtTrId("cutelyst-valbetween-genvalerr-string").arg(min, max);
        } else {
            //: %1 will be replaced by the minimum, %2 by the maximum value
            //% "The value must be between %1 and %2."
            return c->qtTrId("cutelyst-valbetween-genvalerr-num").arg(min, max);
        }
    } else {
        if (d->type == QMetaType::QString) {
            //: %1 will be replaced by the field label, %2 by the minimum and %3 by the
            //: maximum value
            //% "The text in the “%1“ field must be between %2 and %3 characters long."
            return c->qtTrId("cutelyst-valbetween-genvalerr-string-label").arg(_label, min, max);
        } else {
            //: %1 will be replaced by the field label, %2 by the minimum and %3 by the
            //: maximum value
            //% "The value in the “%1” field must be between %2 and %3."
            return c->qtTrId("cutelyst-valbetween-genvalerr-num-label").arg(_label, min, max);
        }
    }

    return {};
}

QString ValidatorBetween::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    const auto errorType = static_cast<ValidatorRulePrivate::ErrorType>(errorData.toInt());
    const QString _label = label(c);

    if (_label.isEmpty()) {
        switch (errorType) {
        case ValidatorRulePrivate::ErrorType::InvalidMin:
            //% "The minimum comparison value is not valid."
            return c->qtTrId("cutelyst-validator-genvaldataerr-min");
        case ValidatorRulePrivate::ErrorType::InvalidType:
        {
            Q_D(const ValidatorBetween);
            QMetaType _type(d->type);
            //: %1 will be replaced by the name of the comparison type
            //% "The comparison type %1 is not supported."
            return c->qtTrId("cutelyst-validator-genvaldataerr-type")
                .arg(QString::fromLatin1(_type.name()));
        }
        case ValidatorRulePrivate::ErrorType::InvalidMax:
            //% "The maximum comparison value is not valid."
            return c->qtTrId("cutelyst-validator-genvaldataerr-max");
        }
    } else {
        switch (errorType) {
        case ValidatorRulePrivate::ErrorType::InvalidMin:
            //: %1 will be replaced by the field label
            //% "The minimum comparison value for the “%1” field is not valid."
            return c->qtTrId("cutelyst-validator-genvaldataerr-min-label").arg(_label);
        case ValidatorRulePrivate::ErrorType::InvalidType:
        {
            Q_D(const ValidatorBetween);
            QMetaType _type(d->type);
            //: %1 will be replaced by the type name, %2 will be replaced by the field label
            //% "The comparison type %1 for the “%2” field is not supported."
            return c->qtTrId("cutelyst-validator-genvaldataerr-type-label")
                .arg(QString::fromLatin1(_type.name()), _label);
        }
        case ValidatorRulePrivate::ErrorType::InvalidMax:
            //: %1 will be replaced by the field label
            //% "The maximum comparison value for the “%1” field is not valid."
            return c->qtTrId("cutelyst-validator-genvaldataerr-max-label").arg(_label);
        }
    }

    return {};
}

QString ValidatorBetween::genericParsingError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    Q_D(const ValidatorBetween);

    const QString _label = label(c);
    if ((d->type == QMetaType::Float) || (d->type == QMetaType::Double)) {
        if (_label.isEmpty()) {
            //% "Failed to parse the input value into a floating point number."
            return c->qtTrId("cutelyst-validator-genparseerr-float");
        } else {
            //: %1 will be replaced by the field label
            //% "Failed to parse the input value for the “%1” field into a "
            //% "floating point number."
            return c->qtTrId("cutelyst-validator-genparseerr-float-label").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            //% "Failed to parse the input value into an integer number."
            return c->qtTrId("cutelyst-validator-genparseerr-int");
        } else {
            //: %1 will be replaced by the field label
            //% "Failed to parse the input value for the “%1” field into an integer number."
            return c->qtTrId("cutelyst-validator-genparseerr-int-label").arg(_label);
        }
    }
}
