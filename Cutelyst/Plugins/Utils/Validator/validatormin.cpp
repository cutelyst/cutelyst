/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatormin_p.h"

using namespace Cutelyst;

ValidatorMin::ValidatorMin(const QString &field,
                           QMetaType::Type type,
                           const QVariant &min,
                           const Cutelyst::ValidatorMessages &messages,
                           const QString &defValKey)
    : ValidatorRule(*new ValidatorMinPrivate(field, type, min, messages, defValKey))
{
}

ValidatorMin::~ValidatorMin() = default;

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
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " Failed to parse \"" << v << "\" into an integer number";
            } else {
                const qlonglong min = d->extractLongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid minimum comparison value";
                } else {
                    if (val < min) {
                        result.errorMessage = validationError(c, min);
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << val << "is not greater than" << min;
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
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " Failed to parse \"" << v
                    << "\" into an unsigned integer number";
            } else {
                const qulonglong min = d->extractULongLong(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid minimum comparison value";
                } else {
                    if (val < min) {
                        result.errorMessage = validationError(c, min);
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << val << "is not greater than" << min;
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
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " Failed to parse \"" << v
                    << "\" into a floating point number";
            } else {
                const double min = d->extractDouble(c, params, d->min, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid minimum comparison value";
                } else {
                    if (val < min) {
                        result.errorMessage = validationError(c, min);
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << val << "is not greater than" << min;
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::QString:
        {
            const auto val      = static_cast<qlonglong>(v.length());
            const qlonglong min = d->extractLongLong(c, params, d->min, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = validationDataError(
                    c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMin));
                qCWarning(C_VALIDATOR).noquote()
                    << debugString(c) << "Invalid minimum comparison value";
            } else {
                if (val < min) {
                    result.errorMessage = validationError(c, min);
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "String length" << val << "is not longer than" << min;
                } else {
                    valid = true;
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
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorMin::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorMin);

    QString min;
    switch (d->type) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        min = c->locale().toString(errorData.toLongLong());
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        min = c->locale().toString(errorData.toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        min = c->locale().toString(errorData.toDouble());
        break;
    default:
        return validationDataError(c,
                                   static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidType));
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            //% "The text must be longer than %1 characters."
            return c->qtTrId("cutelyst-valmin-genvalerr-str").arg(min);
        } else {
            //% "The value must be greater than %1."
            return c->qtTrId("cutelyst-valmin-genvalerr-num").arg(min);
        }
    } else {
        if (d->type == QMetaType::QString) {
            //% "The text in the “%1“ field must be longer than %2 characters."
            return c->qtTrId("cutelyst-valmin-genvalerr-str-label").arg(_label, min);
        } else {
            //% "The value in the “%1” field must be greater than %2."
            return c->qtTrId("cutelyst-valmin-genvalerr-num-label").arg(_label, min);
        }
    }
}

QString ValidatorMin::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    const QString _label = label(c);
    const auto errorType = static_cast<ValidatorRulePrivate::ErrorType>(errorData.toInt());

    // translation strings are defined in ValidatorBetween

    if (_label.isEmpty()) {
        switch (errorType) {
        case ValidatorRulePrivate::ErrorType::InvalidType:
        {
            Q_D(const ValidatorMin);
            const QMetaType _type(d->type);
            return c->qtTrId("cutelyst-validator-genvaldataerr-type")
                .arg(QString::fromLatin1(_type.name()));
        }
        case ValidatorRulePrivate::ErrorType::InvalidMin:
            return c->qtTrId("cutelyst-validator-genvaldataerr-min");
        case ValidatorRulePrivate::ErrorType::InvalidMax:
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
            Q_UNREACHABLE();
            return {};
        }
    } else {
        switch (errorType) {
        case ValidatorRulePrivate::ErrorType::InvalidType:
        {
            Q_D(const ValidatorMin);
            const QMetaType _type(d->type);
            return c->qtTrId("cutelyst-validator-genvaldataerr-type-label")
                .arg(QString::fromLatin1(_type.name()), _label);
        }
        case ValidatorRulePrivate::ErrorType::InvalidMin:
            return c->qtTrId("cutelyst-validator-genvaldataerr-min-label").arg(_label);
        case ValidatorRulePrivate::ErrorType::InvalidMax:
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
            Q_UNREACHABLE();
            return {};
        }
    }

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
    Q_UNREACHABLE_RETURN({});
#else
    return {};
#endif
}

QString ValidatorMin::genericParsingError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    Q_D(const ValidatorMin);

    // translation strings are defined in ValidatorBetween

    const QString _label = label(c);
    if ((d->type == QMetaType::Float) || (d->type == QMetaType::Double)) {
        if (_label.isEmpty()) {
            return c->qtTrId("cutelyst-validator-genparseerr-float");
        } else {
            return c->qtTrId("cutelyst-validator-genparseerr-float-label").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            return c->qtTrId("cutelyst-validator-genparseerr-int");
        } else {
            return c->qtTrId("cutelyst-validator-genparseerr-int-label").arg(_label);
        }
    }
}
