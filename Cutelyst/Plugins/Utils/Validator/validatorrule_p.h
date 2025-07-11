/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORRULE_P_H
#define CUTELYSTVALIDATORRULE_P_H

#include "validatorrule.h"

#include <Cutelyst/Context>
#include <limits>

#include <QDate>
#include <QDateTime>
#include <QLocale>
#include <QTime>
#include <QTimeZone>

namespace Cutelyst {

class ValidatorRulePrivate
{
    Q_DISABLE_COPY(ValidatorRulePrivate)
public:
    ValidatorRulePrivate() = default;

    ValidatorRulePrivate(const QString &f,
                         const ValidatorMessages &m,
                         const QString &dvk,
                         QByteArrayView valName)
        : field(f)
        , defValKey(dvk)
        , messages(m)
        , validatorName(valName)
    {
    }

    virtual ~ValidatorRulePrivate() = default;

    QDate extractDate(Context *c, const QString &date, const char *format = nullptr) const
    {
        QDate d;

        Q_ASSERT(c);

        if (format) {
            const QString _format =
                translationContext ? c->translate(translationContext, format) : c->qtTrId(format);
            d = QDate::fromString(date, _format);
            if (d.isValid()) {
                return d;
            }
            d = c->locale().toDate(date, _format);
            if (d.isValid()) {
                return d;
            }
        }

        for (QLocale::FormatType f : {QLocale::ShortFormat, QLocale::LongFormat}) {
            d = c->locale().toDate(date, f);
            if (d.isValid()) {
                return d;
            }
        }

        for (Qt::DateFormat f : {Qt::ISODate, Qt::RFC2822Date, Qt::TextDate}) {
            d = QDate::fromString(date, f);
            if (d.isValid()) {
                return d;
            }
        }

        return d;
    }

    QTime extractTime(Context *c, const QString &time, const char *format = nullptr) const
    {
        QTime t;

        Q_ASSERT(c);

        if (format) {
            const QString _format =
                translationContext ? c->translate(translationContext, format) : c->qtTrId(format);
            t = QTime::fromString(time, _format);
            if (t.isValid()) {
                return t;
            }
            t = c->locale().toTime(time, _format);
            if (t.isValid()) {
                return t;
            }
        }

        for (QLocale::FormatType f : {QLocale::ShortFormat, QLocale::LongFormat}) {
            t = c->locale().toTime(time, f);
            if (t.isValid()) {
                return t;
            }
        }

        for (Qt::DateFormat f : {Qt::ISODate, Qt::RFC2822Date, Qt::TextDate}) {
            t = QTime::fromString(time, f);
            if (t.isValid()) {
                return t;
            }
        }

        return t;
    }

    QDateTime extractDateTime(Context *c,
                              const QString &dateTime,
                              const char *format  = nullptr,
                              const QTimeZone &tz = QTimeZone()) const
    {
        QDateTime dt;

        Q_ASSERT(c);

        if (format) {
            const QString _format =
                translationContext ? c->translate(translationContext, format) : c->qtTrId(format);
            dt = QDateTime::fromString(dateTime, _format);
            if (dt.isValid()) {
                if (tz.isValid()) {
                    dt.setTimeZone(tz);
                }
                return dt;
            }

            dt = c->locale().toDateTime(dateTime, _format);
            if (dt.isValid()) {
                if (tz.isValid()) {
                    dt.setTimeZone(tz);
                }
                return dt;
            }
        }

        for (QLocale::FormatType f : {QLocale::ShortFormat, QLocale::LongFormat}) {
            dt = c->locale().toDateTime(dateTime, f);
            if (dt.isValid()) {
                if (tz.isValid()) {
                    dt.setTimeZone(tz);
                }
                return dt;
            }
        }

        for (Qt::DateFormat f : {Qt::ISODate, Qt::RFC2822Date, Qt::TextDate}) {
            dt = QDateTime::fromString(dateTime, f);
            if (dt.isValid()) {
                if (tz.isValid()) {
                    dt.setTimeZone(tz);
                }
                return dt;
            }
        }

        return dt;
    }

    QVariant extractOtherDateTime(Context *c,
                                  const ParamsMultiMap &params,
                                  const QString &field,
                                  const QTimeZone &tz = QTimeZone(),
                                  const char *format  = nullptr) const
    {
        QVariant var;

        Q_ASSERT(c);

        qsizetype sepPos = field.indexOf(u'|');
        if (sepPos > -1) {
            const QString fieldName = field.left(sepPos);
            const QString value     = params.value(fieldName);
            if (!value.isEmpty()) {
                const QString type = field.mid(sepPos + 1);
                if (type.startsWith(u"dt")) {
                    QDateTime dt = extractDateTime(c, value, format);
                    if (dt.isValid()) {
                        if (tz.isValid()) {
                            dt.setTimeZone(tz);
                        }
                        var = dt;
                    }
                } else if (type.startsWith(u't')) {
                    const QTime t = extractTime(c, value, format);
                    if (t.isValid()) {
                        var = t;
                    }
                } else if (type.startsWith(u'd')) {
                    const QDate d = extractDate(c, value, format);
                    if (d.isValid()) {
                        var = d;
                    }
                }
            }
        } else {
            var = c->stash(field);
        }

        return var;
    }

    /**
     * @internal
     * Returns a time zone either extracted directly from the value of \a field or from value in
     * \a params identified by the value of \a field as key. If there is no suche key in \a params,
     * it will be looked into the stash if there is a value identified by \a field. You should check
     * the returned QTimeZone for validity.
     */
    static QTimeZone extractTimeZone(Context *c, const ParamsMultiMap &params, const QString &field)
    {
        QTimeZone tz;

        Q_ASSERT(c);
        Q_UNUSED(params)

        tz = QTimeZone(field.toLatin1());

        if (!tz.isValid()) {
            const QString tzString = params.value(field, c->stash(field).toString());
            //            const QString tzString = c->stash(field).toString();
            if (!tzString.isEmpty()) {
                tz = QTimeZone(tzString.toLatin1());
                if (!tz.isValid()) {
                    tz = QTimeZone(tzString.toInt());
                }
            }
        }

        return tz;
    }

    static qlonglong
        extractLongLong(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        qlonglong val = 0;

        Q_ASSERT(c);
        Q_ASSERT(ok);
        Q_UNUSED(params)

        if (value.userType() == QMetaType::QString) {
            const QString field = value.toString();
            /*            if (params.contains(field)) {
                            const QString v = params.value(field);
                            val = v.toLongLong(ok);
            //                if (!*ok) {
            //                    val = c->locale().toLongLong(v, ok);
            //                }
                        } else */
            if (c->stash().contains(field)) {
                val = c->stash(field).toLongLong(ok);
            } else {
                *ok = false;
            }
        } else {
            val = value.toLongLong(ok);
        }

        return val;
    }

    static qulonglong
        extractULongLong(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        qulonglong val = 0;

        Q_ASSERT(c);
        Q_ASSERT(ok);
        Q_UNUSED(params)

        if (value.userType() == QMetaType::QString) {
            const QString field = value.toString();
            /*            if (params.contains(field)) {
                            const QString v = params.value(field);
                            val = v.toULongLong(ok);
            //                if (!*ok) {
            //                    val = c->locale().toULongLong(v, ok);
            //                }
                        } else */
            if (c->stash().contains(field)) {
                val = c->stash(field).toULongLong(ok);
            } else {
                *ok = false;
            }
        } else {
            val = value.toULongLong(ok);
        }

        return val;
    }

    static double
        extractDouble(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        double val = 0.0;

        Q_ASSERT(c);
        Q_ASSERT(ok);
        Q_UNUSED(params)

        if (value.userType() == QMetaType::QString) {
            const QString field = value.toString();
            /*            if (params.contains(field)) {
                            const QString v = params.value(field);
                            val = v.toDouble(ok);
            //                if (!*ok) {
            //                    val = c->locale().toDouble(v, ok);
            //                }
                        } else*/
            if (c->stash().contains(field)) {
                val = c->stash(field).toDouble(ok);
            } else {
                *ok = false;
            }
        } else {
            val = value.toDouble(ok);
        }

        return val;
    }

    static int extractInt(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        int val = 0;

        Q_ASSERT(c);
        Q_ASSERT(ok);
        Q_UNUSED(params)

        if (value.userType() == QMetaType::QString) {
            const QString field = value.toString();
            /*            if (params.contains(field)) {
                            const QString _val = params.value(field);
                            val = _val.toInt(ok);
            //                if (!*ok) {
            //                    val = c->locale().toInt(_val, ok);
            //                }
                        } else*/
            if (c->stash().contains(field)) {
                val = c->stash(field).toInt(ok);
            } else {
                *ok = false;
            }
        } else {
            val = value.toInt(ok);
        }

        return val;
    }

    static qsizetype
        extractSizeType(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        return extractLongLong(c, params, value, ok);
    }

    static QVariant valueToNumber(Context *c, const QString &value, QMetaType::Type type)
    {
        QVariant var;

        Q_UNUSED(c)

        bool ok = false;

        switch (type) {
        case QMetaType::Char:
        {
            const short _v = value.toShort(&ok);
            if (ok) {
                if ((_v < static_cast<short>(std::numeric_limits<char>::max())) &&
                    (_v > static_cast<short>(std::numeric_limits<char>::min()))) {
                    var.setValue<char>(static_cast<char>(_v));
                }
            }
        } break;
        case QMetaType::Short:
        {
            //            const short v = c->locale().toShort(value, &ok);
            const short v = value.toShort(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::Int:
        {
            //            const int v = c->locale().toInt(value, &ok);
            const int v = value.toInt(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::Long:
        {
            const long v = value.toLong(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::LongLong:
        {
            //            const qlonglong v = c->locale().toLongLong(value, &ok);
            //            if (ok) {
            //                if (type == QMetaType::Long) {
            //                    if (v < static_cast<qlonglong>(std::numeric_limits<long>::max()))
            //                    {
            //                        var.setValue<long>(static_cast<long>(v));
            //                    }
            //                } else {
            //                    var.setValue<qlonglong>(v);
            //                }
            //            }
            const qlonglong v = value.toLongLong(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::UChar:
        {
            const ushort _v = value.toUShort(&ok);
            if (ok) {
                if ((_v < static_cast<ushort>(std::numeric_limits<uchar>::max())) &&
                    (_v > static_cast<ushort>(std::numeric_limits<uchar>::min()))) {
                    var.setValue<uchar>(static_cast<uchar>(_v));
                }
            }
        } break;
        case QMetaType::UShort:
        {
            //            const ushort v = c->locale().toUShort(value, &ok);
            const ushort v = value.toUShort(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::UInt:
        {
            //            const uint v = c->locale().toUInt(value, &ok);
            const uint v = value.toUInt(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::ULong:
        {
            const ulong v = value.toULong(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::ULongLong:
        {
            //            const qulonglong v = c->locale().toULongLong(value, &ok);
            //            if (ok) {
            //                if (type == QMetaType::ULong) {
            //                    if ((v >
            //                    static_cast<qulonglong>(std::numeric_limits<ulong>::min())) && (v
            //                    < static_cast<qulonglong>(std::numeric_limits<ulong>::max()))) {
            //                        var.setValue<ulong>(static_cast<ulong>(v));
            //                    }
            //                } else {
            //                    var.setValue<qulonglong>(v);
            //                }
            //            }
            const qulonglong v = value.toULongLong(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::Float:
        {
            //            const float v = c->locale().toFloat(value, &ok);
            const float v = value.toFloat(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        case QMetaType::Double:
        {
            //            const double v = c->locale().toDouble(value, &ok);
            const double v = value.toDouble(&ok);
            if (ok) {
                var.setValue(v);
            }
        } break;
        default:
            break;
        }

        return var;
    }

    // decimal ASCII codes used by some validators
    static constexpr char16_t asciiTab{9};
    static constexpr char16_t asciiSpace{32};
    static constexpr char16_t ascii_dash{45};
    static constexpr char16_t ascii_0{48};
    static constexpr char16_t ascii_9{57};
    static constexpr char16_t ascii_A{65};
    static constexpr char16_t ascii_Z{90};
    static constexpr char16_t ascii_underscore{95};
    static constexpr char16_t ascii_a{97};
    static constexpr char16_t ascii_z{122};

    // used by some validators for generic data errors
    enum class ErrorType : int { InvalidMin, InvalidMax, InvalidType };

    const char *translationContext{nullptr};
    QString field;
    QString defValKey;
    ValidatorMessages messages;
    QByteArrayView validatorName;
    bool trimBefore{true};
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORRULE_P_H
