/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatormax_p.h"

#include <QMetaType>

using namespace Cutelyst;

ValidatorMax::ValidatorMax(const QString &field,
                           QMetaType::Type type,
                           const QVariant &max,
                           const Cutelyst::ValidatorMessages &messages,
                           const QString &defValKey)
    : ValidatorRule(*new ValidatorMaxPrivate(field, type, max, messages, defValKey))
{
}

ValidatorMax::~ValidatorMax() = default;

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
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << " Failed to parse \"" << v << "\" into an integer number";
            } else {
                const qlonglong max = d->extractLongLong(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid maximum comparison value";
                } else {
                    if (val > max) {
                        result.errorMessage = validationError(c, max);
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << val << "is not smaller than" << max;
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
                const qulonglong max = d->extractULongLong(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid maximum comparison value";
                } else {
                    if (val > max) {
                        result.errorMessage = validationError(c, max);
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << val << "is not smaller than" << max;
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
                const double max = d->extractDouble(c, params, d->max, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(
                        c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                    qCWarning(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid maximum comparison value";
                } else {
                    if (val > max) {
                        result.errorMessage = validationError(c, max);
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << val << "is not smaller than" << max;
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::QString:
        {
            const auto val      = static_cast<qlonglong>(v.length());
            const qlonglong max = d->extractLongLong(c, params, d->max, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = validationDataError(
                    c, static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidMax));
                qCWarning(C_VALIDATOR).noquote()
                    << debugString(c) << "Invalid maximum comparison value";
            } else {
                if (val > max) {
                    result.errorMessage = validationError(c, max);
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "String length" << val << "is not shorter than" << max;
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

QString ValidatorMax::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorMax);

    QString max;
    switch (d->type) {
    case QMetaType::Char:
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        max = c->locale().toString(errorData.toLongLong());
        break;
    case QMetaType::UChar:
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        max = c->locale().toString(errorData.toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        max = c->locale().toString(errorData.toDouble());
        break;
    default:
        return validationDataError(c,
                                   static_cast<int>(ValidatorRulePrivate::ErrorType::InvalidType));
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            //% "The text must be shorter than %1 characters."
            return c->qtTrId("cutelyst-valmax-genvalerr-str").arg(max);
        } else {
            //% "The value must be lower than %1."
            return c->qtTrId("cutelyst-valmax-genvalerr-num").arg(max);
        }
    } else {
        if (d->type == QMetaType::QString) {
            //% "The text in the “%1“ field must be shorter than %2 characters."
            return c->qtTrId("cutelyst-valmax-genvalerr-str-label").arg(_label, max);
        } else {
            //% "The value in the “%1” field must be lower than %2."
            return c->qtTrId("cutelyst-valmax-genvalerr-num-label").arg(_label, max);
        }
    }
}

QString ValidatorMax::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    const QString _label = label(c);
    const auto errorType = static_cast<ValidatorRulePrivate::ErrorType>(errorData.toInt());

    // translation strings are defined in ValidatorBetween

    if (_label.isEmpty()) {
        switch (errorType) {
        case ValidatorRulePrivate::ErrorType::InvalidType:
        {
            Q_D(const ValidatorMax);
            const QMetaType _type(d->type);
            return c->qtTrId("cutelyst-validator-genvaldataerr-type")
                .arg(QString::fromLatin1(_type.name()));
        }
        case ValidatorRulePrivate::ErrorType::InvalidMax:
            return c->qtTrId("cutelyst-validator-genvaldataerr-max");
        case ValidatorRulePrivate::ErrorType::InvalidMin:
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-do-while)
            Q_UNREACHABLE();
            return {};
        }
    } else {
        switch (errorType) {
        case ValidatorRulePrivate::ErrorType::InvalidType:
        {
            Q_D(const ValidatorMax);
            const QMetaType _type(d->type);
            return c->qtTrId("cutelyst-validator-genvaldataerr-type-label")
                .arg(QString::fromLatin1(_type.name()), _label);
        }
        case ValidatorRulePrivate::ErrorType::InvalidMax:
            return c->qtTrId("cutelyst-validator-genvaldataerr-max-label").arg(_label);
        case ValidatorRulePrivate::ErrorType::InvalidMin:
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

QString ValidatorMax::genericParsingError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    Q_D(const ValidatorMax);

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
