/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef CUTELYSTVALIDATORRULE_P_H
#define CUTELYSTVALIDATORRULE_P_H

#include "validatorrule.h"
#include <QDate>
#include <QTime>
#include <QDateTime>

namespace Cutelyst {

class ValidatorRulePrivate
{
public:
    ValidatorRulePrivate() :
        valid(false),
        parsingError(false),
        validationDataError(false),
        trimBefore(true)
    {}

    ValidatorRulePrivate(const QString f, const QString &l, const QString &e) :
        field(f),
        label(l),
        customError(e),
        valid(false),
        parsingError(false),
        validationDataError(false),
        trimBefore(true)
    {}

    virtual ~ValidatorRulePrivate() {}

    QDate extractDate(const QString &date, const QString &format = QString()) const
    {
        QDate d;

        if (!format.isEmpty()) {
            d = QDate::fromString(date, format);
            if (d.isValid()) {
                return d;
            }
        }

        const QList<Qt::DateFormat> fs({Qt::ISODate, Qt::RFC2822Date, Qt::TextDate});

        for (Qt::DateFormat f : fs) {
            d = QDate::fromString(date, f);
            if (d.isValid()) {
                return d;
            }
        }

        return d;
    }


    QTime extractTime(const QString &time, const QString &format = QString()) const
    {
        QTime t;

        if (!format.isEmpty()) {
            t = QTime::fromString(time, format);
            if (t.isValid()) {
                return t;
            }
        }

        const QList<Qt::DateFormat> fs({Qt::ISODate, Qt::RFC2822Date, Qt::TextDate});

        for (Qt::DateFormat f : fs) {
            t = QTime::fromString(time, f);
            if (t.isValid()) {
                return t;
            }
        }

        return t;
    }


    QDateTime extractDateTime(const QString &dateTime, const QString &format) const
    {
        QDateTime dt;

        if (!format.isEmpty()) {
            dt = QDateTime::fromString(dateTime, format);
            if (dt.isValid()) {
                return dt;
            }
        }

        const QList<Qt::DateFormat> fs({Qt::ISODate, Qt::RFC2822Date, Qt::TextDate});

        for (Qt::DateFormat f : fs) {
            dt = QDateTime::fromString(dateTime, f);
            if (dt.isValid()) {
                return dt;
            }
        }

        return dt;
    }



    QMetaType::Type getBaseType(QMetaType::Type type) const
    {
        switch (type) {
            case QMetaType::Int:
            case QMetaType::Long:
            case QMetaType::LongLong:
            case QMetaType::Short:
            case QMetaType::Char:
            case QMetaType::SChar:
                return QMetaType::Int;
            case QMetaType::UInt:
            case QMetaType::ULong:
            case QMetaType::ULongLong:
            case QMetaType::UShort:
            case QMetaType::UChar:
                return QMetaType::UInt;
            case QMetaType::Float:
            case QMetaType::Double:
                return QMetaType::Float;
            case QMetaType::QString:
                return QMetaType::QString;
            case QMetaType::QVariantList:
            case QMetaType::QStringList:
            case QMetaType::QVariantMap:
            case QMetaType::QVariantHash:
            case QMetaType::QByteArrayList:
                return QMetaType::QVariantList;
            default:
                return QMetaType::UnknownType;
        }
    }

    QString field;
    QString label;
    QString customError;
    bool valid;
    ParamsMultiMap parameters;
    bool parsingError;
    bool validationDataError;
    QString customParsingError;
    QString customValidationDataError;
    bool trimBefore;
};

}

#endif //CUTELYSTVALIDATORRULE_P_H

