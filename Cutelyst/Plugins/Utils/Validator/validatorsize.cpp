/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorsize_p.h"

using namespace Cutelyst;

ValidatorSize::ValidatorSize(const QString &field,
                             QMetaType::Type type,
                             const QVariant &size,
                             const Cutelyst::ValidatorMessages &messages,
                             const QString &defValKey)
    : ValidatorRule(*new ValidatorSizePrivate(field, type, size, messages, defValKey))
{
}

ValidatorSize::~ValidatorSize() = default;

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
            const auto val = c->locale().toLongLong(v, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << "Failed to parse \"" << v << "\" into an integer number";
            } else {
                const qlonglong size = d->extractLongLong(c, params, d->size, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid comparison size";
                } else {
                    if (val != size) {
                        result.errorMessage = validationError(c, size);
                        qCDebug(C_VALIDATOR).noquote() << debugString(c) << val << "!=" << size;
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
            const auto val = v.toULongLong(&ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << "Failed to parse \"" << v
                    << "\" into an unsigned integer number";
            } else {
                const qulonglong size = d->extractULongLong(c, params, d->size, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid comparison size";
                } else {
                    if (val != size) {
                        result.errorMessage = validationError(c, size);
                        qCDebug(C_VALIDATOR).noquote() << debugString(c) << val << "!=" << size;
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::Float:
        case QMetaType::Double:
        {
            const auto val = v.toDouble(&ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = parsingError(c);
                qCWarning(C_VALIDATOR).noquote().nospace()
                    << debugString(c) << "Failed to parse \"" << v
                    << "\" into a floating point number";
            } else {
                const double size = d->extractDouble(c, params, d->size, &ok);
                if (Q_UNLIKELY(!ok)) {
                    result.errorMessage = validationDataError(c, 1);
                    qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid comparison size";
                } else {
                    if (val != size) {
                        result.errorMessage = validationError(c, size);
                        qCDebug(C_VALIDATOR).noquote() << debugString(c) << val << "!=" << size;
                    } else {
                        valid = true;
                    }
                }
            }
        } break;
        case QMetaType::QString:
        {
            const auto val       = static_cast<qlonglong>(v.length());
            const qlonglong size = d->extractLongLong(c, params, d->size, &ok);
            if (Q_UNLIKELY(!ok)) {
                result.errorMessage = validationDataError(c, 1);
                qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid comparison size";
            } else {
                if (val != size) {
                    result.errorMessage = validationError(c, size);
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "string length" << val << "!=" << size;
                } else {
                    valid = true;
                }
            }
        } break;
        default:
            qCWarning(C_VALIDATOR).noquote()
                << debugString(c) << "The comparison type" << d->type << "is not supported";
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
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorSize::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorSize);

    QString size;
    switch (d->type) {
    case QMetaType::Short:
    case QMetaType::Int:
    case QMetaType::Long:
    case QMetaType::LongLong:
    case QMetaType::QString:
        size = c->locale().toString(errorData.toLongLong());
        break;
    case QMetaType::UShort:
    case QMetaType::UInt:
    case QMetaType::ULong:
    case QMetaType::ULongLong:
        size = c->locale().toString(errorData.toULongLong());
        break;
    case QMetaType::Float:
    case QMetaType::Double:
        size = c->locale().toString(errorData.toDouble());
        break;
    default:
        return validationDataError(c, 0);
    }

    const QString _label = label(c);

    if (_label.isEmpty()) {
        if (d->type == QMetaType::QString) {
            //% "The text must be exactly %1 characters long."
            return c->qtTrId("cutelyst-valsize-genvalerr-str").arg(size);
        } else {
            //% "The value must be %1."
            return c->qtTrId("cutelyst-valsize-genvalerr-num").arg(size);
        }
    } else {
        if (d->type == QMetaType::QString) {
            //: %1 will be replaced by the field label, %2 will be replaced by the required string
            //: size
            //% "The text in the “%1“ field must be exactly %2 characters long."
            return c->qtTrId("cutelyst-valsize-genvalerr-str-label").arg(_label, size);
        } else {
            //: %1 will be replaced by the field label, %2 will be replaced by the required
            //: size/value
            //% "The value in the “%1” field must be %2."
            return c->qtTrId("cutelyst-valsize-genvalerr-num-label").arg(_label, size);
        }
    }
}

QString ValidatorSize::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    int field            = errorData.toInt();
    const QString _label = label(c);

    if (field == 0) {
        Q_D(const ValidatorSize);
        const QMetaType _type(d->type);
        if (_label.isEmpty()) {
            return c->qtTrId("cutelyst-validator-genvaldataerr-type")
                .arg(QString::fromLatin1(_type.name()));
        } else {
            return c->qtTrId("cutelyst-validator-genvaldataerr-type-label")
                .arg(QString::fromLatin1(_type.name()), _label);
        }
    } else {
        if (_label.isEmpty()) {
            //% "The comparison value is not valid."
            return c->qtTrId("cutelyst-valsize-genvaldataerr-size");
        } else {
            //: %1 will be replaced by the field label
            //% "The comparison value for the “%1” field is not valid."
            return c->qtTrId("cutelyst-valsize-genvaldataerr-size-label").arg(_label);
        }
    }
}

QString ValidatorSize::genericParsingError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    Q_D(const ValidatorSize);

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
