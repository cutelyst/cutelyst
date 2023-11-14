/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorinteger_p.h"

using namespace Cutelyst;

ValidatorInteger::ValidatorInteger(const QString &field,
                                   QMetaType::Type type,
                                   const Cutelyst::ValidatorMessages &messages,
                                   const QString &defValKey)
    : ValidatorRule(*new ValidatorIntegerPrivate(field, type, messages, defValKey))
{
}

ValidatorInteger::~ValidatorInteger() = default;

ValidatorReturnType ValidatorInteger::validate(Cutelyst::Context *c,
                                               const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        Q_D(const ValidatorInteger);
        QVariant converted;

        switch (d->type) {
        case QMetaType::Char:
        case QMetaType::Short:
        case QMetaType::Int:
        case QMetaType::Long:
        case QMetaType::LongLong:
        case QMetaType::UChar:
        case QMetaType::UShort:
        case QMetaType::UInt:
        case QMetaType::ULong:
        case QMetaType::ULongLong:
            converted = d->valueToNumber(c, v, d->type);
            break;
        default:
            result.errorMessage = validationDataError(c);
            qCWarning(C_VALIDATOR).noquote()
                << debugString(c) << "Conversion type" << d->type << "is not an integer type";
            break;
        }

        if (converted.isValid()) {
            result.value = converted;
        } else {
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " \"" << v << "\" is not parseable as integer value "
                << "or exceeds the limits of the selected type " << d->type;
            result.errorMessage = validationError(c);
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorInteger::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    Q_D(const ValidatorInteger);
    const QString _label = label(c);
    QString min;
    QString max;
    switch (d->type) {
    case QMetaType::Char:
        min = c->locale().toString(std::numeric_limits<char>::min());
        max = c->locale().toString(std::numeric_limits<char>::max());
        break;
    case QMetaType::Short:
        min = c->locale().toString(std::numeric_limits<short>::min());
        max = c->locale().toString(std::numeric_limits<short>::max());
        break;
    case QMetaType::Int:
        min = c->locale().toString(std::numeric_limits<int>::min());
        max = c->locale().toString(std::numeric_limits<int>::max());
        break;
    case QMetaType::Long:
        min = c->locale().toString(static_cast<qlonglong>(std::numeric_limits<long>::min()));
        max = c->locale().toString(static_cast<qlonglong>(std::numeric_limits<long>::max()));
        break;
    case QMetaType::LongLong:
        min = c->locale().toString(std::numeric_limits<qlonglong>::min());
        max = c->locale().toString(std::numeric_limits<qlonglong>::max());
        break;
    case QMetaType::UChar:
        min = c->locale().toString(std::numeric_limits<uchar>::min());
        max = c->locale().toString(std::numeric_limits<uchar>::max());
        break;
    case QMetaType::UShort:
        min = c->locale().toString(std::numeric_limits<ushort>::min());
        max = c->locale().toString(std::numeric_limits<ushort>::max());
        break;
    case QMetaType::UInt:
        min = c->locale().toString(std::numeric_limits<uint>::min());
        max = c->locale().toString(std::numeric_limits<uint>::max());
        break;
    case QMetaType::ULong:
        min = c->locale().toString(static_cast<qulonglong>(std::numeric_limits<ulong>::min()));
        max = c->locale().toString(static_cast<qulonglong>(std::numeric_limits<ulong>::max()));
        break;
    case QMetaType::ULongLong:
    default:
        min = c->locale().toString(std::numeric_limits<qulonglong>::min());
        max = c->locale().toString(std::numeric_limits<qulonglong>::max());
        break;
    }
    if (_label.isEmpty()) {
        //: %1 is the minimum numerical limit for the selected type, %2 is the maximum numeric limit
        //% "Not a valid integer value between %1 and %2."
        return c->qtTrId("cutelyst-valinteger-genvalerr").arg(min, max);
    } else {
        //: %1 will be replaced by the field name, %2 is the minimum numerical limit for the
        //: selected type, %3 is the maximum numeric limit
        //% "The value in the “%1“ field is not a valid integer between %2 and %3."
        return c->qtTrId("cutelyst-valinteger-genvalerr-label").arg(_label, min, max);
    }
}
