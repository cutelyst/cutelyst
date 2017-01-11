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

#include "validatorbefore_p.h"

#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_VALIDATORBEFORE, "cutelyst.utils.validator.before")

ValidatorBefore::ValidatorBefore(const QString &field, const QVariant &dateTime, const QString &inputFormat, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorBeforePrivate(field, dateTime, inputFormat, label, customError))
{
}

ValidatorBefore::ValidatorBefore(ValidatorBeforePrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorBefore::~ValidatorBefore()
{
}

QString ValidatorBefore::validate() const
{
    Q_D(const ValidatorBefore);

    QString v = value();

    if (v.isEmpty()) {
        return QString();
    }

    if (d->date.type() == QVariant::Date) {

        QDate odate = d->date.toDate();
        if (!odate.isValid()) {
            qCWarning(C_VALIDATORBEFORE) << "Invalid validation date.";
            return validationDataError();
        }
        QDate date = d->extractDate(v, d->inputFormat);
        if (!date.isValid()) {
            qCWarning(C_VALIDATORBEFORE) << "Can not parse input date:" << v;
            return parsingError();
        }
        if (date < odate) {
            return QString();
        } else {
            return validationError();
        }

    } else if (d->date.type() == QVariant::DateTime) {

        QDateTime odatetime = d->date.toDateTime();
        if (!odatetime.isValid()) {
            qCWarning(C_VALIDATORBEFORE) << "Invalid validation date and time.";
            return validationDataError();
        }
        QDateTime datetime = d->extractDateTime(v, d->inputFormat);
        if (!datetime.isValid()) {
            qCWarning(C_VALIDATORBEFORE) << "Can not parse input date and time:" << v;
            return parsingError();
        }
        if (datetime < odatetime) {
            return QString();
        } else {
            return validationError();
        }

    } else if (d->date.type() == QVariant::Time) {

        QTime otime = d->date.toTime();
        if (!otime.isValid()) {
            qCWarning(C_VALIDATORBEFORE) << "Invalid validation time.";
            return validationDataError();
        }
        QTime time = d->extractTime(v, d->inputFormat);
        if (!time.isValid()) {
            qCWarning(C_VALIDATORBEFORE) << "Can not parse input time:" << v;
            return parsingError();
        }
        if (time < otime) {
            return QString();
        } else {
            return validationError();
        }

    } else {

        qCWarning(C_VALIDATORBEFORE) << "Invalid validation data:" << d->date;
        return validationDataError();

    }

    return validationError();
}

QString ValidatorBefore::genericValidationError() const
{
    Q_D(const ValidatorBefore);

    switch(d->date.type()) {
    case QVariant::Date:
        return QStringLiteral("The date in the “%1” field must be before %2.").arg(fieldLabel(),
                                                                      //: date shown in validator error message
                                                                      d->date.toDate().toString(QStringLiteral("dd.MM.yyyy")));
    case QVariant::DateTime:
        return QStringLiteral("The date and time in the “%1” field must be before %2.").arg(fieldLabel(),
                                                                               //: date and time shown in validator error message
                                                                               d->date.toDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm")));
    case QVariant::Time:
        return QStringLiteral("The time in the “%1” field must be before %2.").arg(fieldLabel(),
                                                                      //: time shown in the validator error message
                                                                      d->date.toTime().toString(QStringLiteral("HH:mm")));
    default:
        return validationDataError();
    }
}

void ValidatorBefore::setDateTime(const QVariant &dateTime)
{
    Q_D(ValidatorBefore);
    d->date = dateTime;
}

void ValidatorBefore::setInputFormat(const QString &format)
{
    Q_D(ValidatorBefore);
    d->inputFormat = format;
}
