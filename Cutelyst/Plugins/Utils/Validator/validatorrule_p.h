/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CUTELYSTVALIDATORRULE_P_H
#define CUTELYSTVALIDATORRULE_P_H

#include "validatorrule.h"
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QTimeZone>
#include <QLocale>
#include <Cutelyst/Context>
#include <limits>

namespace Cutelyst {

class ValidatorRulePrivate
{
    Q_DISABLE_COPY(ValidatorRulePrivate)
public:
    ValidatorRulePrivate() {}

    ValidatorRulePrivate(const QString &f, const ValidatorMessages &m, const QString &dvk) :
        field(f),
        defValKey(dvk),
        messages(m)
    {}

    virtual ~ValidatorRulePrivate() {}

    QDate extractDate(Context *c, const QString &date, const char *format = nullptr) const
    {
        QDate d;

        Q_ASSERT(c);

        if (format) {
            const QString _format = translationContext.size() ? c->translate(translationContext.data(), format) : QString::fromUtf8(format);
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
            const QString _format = translationContext.size() ? c->translate(translationContext.data(), format) : QString::fromUtf8(format);
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


    QDateTime extractDateTime(Context *c, const QString &dateTime, const char *format = nullptr, const QTimeZone &tz = QTimeZone()) const
    {
        QDateTime dt;

        Q_ASSERT(c);

        if (format) {
            const QString _format = translationContext.size() ? c->translate(translationContext.data(), format) : QString::fromUtf8(format);
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

    QVariant extractOtherDateTime(Context *c, const ParamsMultiMap &params, const QString &field, const QTimeZone &tz = QTimeZone(), const char *format = nullptr) const
    {
        QVariant var;

        Q_ASSERT(c);

        int sepPos = field.indexOf(QLatin1Char('|'));
        if (sepPos > -1) {
            const QString fieldName = field.left(sepPos);
            const QString value = params.value(fieldName);
            if (!value.isEmpty()) {
                const QStringRef type = field.midRef(sepPos + 1);
                if (type.startsWith(QLatin1String("dt"))) {
                    QDateTime dt = extractDateTime(c, value, format);
                    if (dt.isValid()) {
                        if (tz.isValid()) {
                            dt.setTimeZone(tz);
                        }
                        var.setValue<QDateTime>(dt);
                    }
                } else if (type.startsWith(QLatin1Char('t'))) {
                    const QTime t = extractTime(c, value, format);
                    if (t.isValid()) {
                        var.setValue<QTime>(t);
                    }
                } else if (type.startsWith(QLatin1Char('d'))) {
                    const QDate d = extractDate(c, value, format);
                    if (d.isValid()) {
                        var.setValue<QDate>(d);
                    }
                }
            }
        } else {
            var = c->stash(field);
        }

        return var;
    }

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

    static qlonglong extractLongLong(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        qlonglong val = 0;

        Q_ASSERT(c);
        Q_ASSERT(ok);
        Q_UNUSED(params)

        if (value.type() == QVariant::String) {
            const QString field = value.toString();
/*            if (params.contains(field)) {
                const QString v = params.value(field);
                val = v.toLongLong(ok);
//                if (!*ok) {
//                    val = c->locale().toLongLong(v, ok);
//                }
            } else */if (c->stash().contains(field)) {
                val = c->stash(field).toLongLong(ok);
            } else {
                *ok = false;
            }
        } else {
            val = value.toLongLong(ok);
        }

        return val;
    }

    static qulonglong extractULongLong(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        qulonglong val = 0;

        Q_ASSERT(c);
        Q_ASSERT(ok);
        Q_UNUSED(params)

        if (value.type() == QVariant::String) {
            const QString field = value.toString();
/*            if (params.contains(field)) {
                const QString v = params.value(field);
                val = v.toULongLong(ok);
//                if (!*ok) {
//                    val = c->locale().toULongLong(v, ok);
//                }
            } else */if (c->stash().contains(field)) {
                val = c->stash(field).toULongLong(ok);
            } else {
                *ok = false;
            }
        } else {
            val = value.toULongLong(ok);
        }

        return val;
    }

    static double extractDouble(Context *c, const ParamsMultiMap &params, const QVariant &value, bool *ok)
    {
        double val = 0.0;

        Q_ASSERT(c);
        Q_ASSERT(ok);
        Q_UNUSED(params)

        if (value.type() == QVariant::String) {
            const QString field = value.toString();
/*            if (params.contains(field)) {
                const QString v = params.value(field);
                val = v.toDouble(ok);
//                if (!*ok) {
//                    val = c->locale().toDouble(v, ok);
//                }
            } else*/ if (c->stash().contains(field)) {
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

        if (value.type() == QVariant::String) {
            const QString field = value.toString();
/*            if (params.contains(field)) {
                const QString _val = params.value(field);
                val = _val.toInt(ok);
//                if (!*ok) {
//                    val = c->locale().toInt(_val, ok);
//                }
            } else*/ if (c->stash().contains(field)) {
                val = c->stash(field).toInt(ok);
            } else {
                *ok = false;
            }
        } else {
            val = value.toInt(ok);
        }

        return val;
    }

    static QVariant valueToNumber(Context *c, const QString &value, QMetaType::Type type)
    {
        QVariant var;

        Q_UNUSED(c);

        bool ok = false;

        switch (type) {
        case QMetaType::Char:
        {
            const short _v = value.toShort(&ok);
            if (ok) {
                if ((_v < static_cast<short>(std::numeric_limits<char>::max())) && (_v > static_cast<short>(std::numeric_limits<char>::min()))) {
                    var.setValue<char>(static_cast<char>(_v));
                }
            }
        }
            break;
        case QMetaType::Short:
        {
//            const short v = c->locale().toShort(value, &ok);
            const short v = value.toShort(&ok);
            if (ok) {
                var.setValue<short>(v);
            }
        }
            break;
        case QMetaType::Int:
        {
//            const int v = c->locale().toInt(value, &ok);
            const int v = value.toInt(&ok);
            if (ok) {
                var.setValue<short>(v);
            }
        }
            break;
        case QMetaType::Long:
        {
            const long v = value.toLong(&ok);
            if (ok) {
                var.setValue<long>(v);
            }
        }
            break;
        case QMetaType::LongLong:
        {
//            const qlonglong v = c->locale().toLongLong(value, &ok);
//            if (ok) {
//                if (type == QMetaType::Long) {
//                    if (v < static_cast<qlonglong>(std::numeric_limits<long>::max())) {
//                        var.setValue<long>(static_cast<long>(v));
//                    }
//                } else {
//                    var.setValue<qlonglong>(v);
//                }
//            }
            const qlonglong v = value.toLongLong(&ok);
            if (ok) {
                var.setValue<qlonglong>(v);
            }
        }
            break;
        case QMetaType::UChar:
        {
            const ushort _v = value.toUShort(&ok);
            if (ok) {
                if ((_v < static_cast<ushort>(std::numeric_limits<uchar>::max())) && (_v > static_cast<ushort>(std::numeric_limits<uchar>::min()))) {
                    var.setValue<uchar>(static_cast<uchar>(_v));
                }
            }
        }
            break;
        case QMetaType::UShort:
        {
//            const ushort v = c->locale().toUShort(value, &ok);
            const ushort v = value.toUShort(&ok);
            if (ok) {
                var.setValue<ushort>(v);
            }
        }
            break;
        case QMetaType::UInt:
        {
//            const uint v = c->locale().toUInt(value, &ok);
            const uint v = value.toUInt(&ok);
            if (ok) {
                var.setValue<uint>(v);
            }
        }
            break;
        case QMetaType::ULong:
        {
            const ulong v = value.toULong(&ok);
            if (ok) {
                var.setValue<ulong>(v);
            }
        }
            break;
        case QMetaType::ULongLong:
        {
//            const qulonglong v = c->locale().toULongLong(value, &ok);
//            if (ok) {
//                if (type == QMetaType::ULong) {
//                    if ((v > static_cast<qulonglong>(std::numeric_limits<ulong>::min())) && (v < static_cast<qulonglong>(std::numeric_limits<ulong>::max()))) {
//                        var.setValue<ulong>(static_cast<ulong>(v));
//                    }
//                } else {
//                    var.setValue<qulonglong>(v);
//                }
//            }
            const qulonglong v = value.toULongLong(&ok);
            if (ok) {
                var.setValue<qulonglong>(v);
            }
        }
            break;
        case QMetaType::Float:
        {
//            const float v = c->locale().toFloat(value, &ok);
            const float v = value.toFloat(&ok);
            if (ok) {
                var.setValue<float>(v);
            }
        }
            break;
        case QMetaType::Double:
        {
//            const double v = c->locale().toDouble(value, &ok);
            const double v = value.toDouble(&ok);
            if (ok) {
                var.setValue<double>(v);
            }
        }
            break;
        default:
            break;
        }

        return var;
    }

    QLatin1String translationContext;
    QString field;
    QString defValKey;
    ValidatorMessages messages;
    bool trimBefore = true;
};

}

#endif //CUTELYSTVALIDATORRULE_P_H

