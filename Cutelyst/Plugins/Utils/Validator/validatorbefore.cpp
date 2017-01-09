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

ValidatorBefore::ValidatorBefore(const QString &field, const QVariant &dateTime, const QString &inputFormat, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorBeforePrivate(field, dateTime, inputFormat, label, customError), parent)
{

}


ValidatorBefore::ValidatorBefore(ValidatorBeforePrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorBefore::~ValidatorBefore()
{

}



bool ValidatorBefore::validate()
{
    Q_D(ValidatorBefore);

    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    if (d->date.type() == QVariant::Date) {

        QDate odate = d->date.toDate();
        if (!odate.isValid()) {
            setValidationDataError(true);
            qCWarning(C_VALIDATORBEFORE) << "Invalid validation date.";
            return false;
        }
        QDate date = d->extractDate(v, d->inputFormat);
        if (!date.isValid()) {
            setParsingError(true);
            qCWarning(C_VALIDATORBEFORE) << "Can not parse input date:" << v;
            return false;
        }
        if (date < odate) {
            setValid(true);
            return true;
        } else {
            return false;
        }

    } else if (d->date.type() == QVariant::DateTime) {

        QDateTime odatetime = d->date.toDateTime();
        if (!odatetime.isValid()) {
            setValidationDataError(true);
            qCWarning(C_VALIDATORBEFORE) << "Invalid validation date and time.";
            return false;
        }
        QDateTime datetime = d->extractDateTime(v, d->inputFormat);
        if (!datetime.isValid()) {
            setParsingError(true);
            qCWarning(C_VALIDATORBEFORE) << "Can not parse input date and time:" << v;
            return false;
        }
        if (datetime < odatetime) {
            setValid(true);
            return true;
        } else {
            return false;
        }

    } else if (d->date.type() == QVariant::Time) {

        QTime otime = d->date.toTime();
        if (!otime.isValid()) {
            setValidationDataError(true);
            qCWarning(C_VALIDATORBEFORE) << "Invalid validation time.";
            return false;
        }
        QTime time = d->extractTime(v, d->inputFormat);
        if (!time.isValid()) {
            setParsingError(true);
            qCWarning(C_VALIDATORBEFORE) << "Can not parse input time:" << v;
            return false;
        }
        if (time < otime) {
            setValid(true);
            return true;
        } else {
            return false;
        }

    } else {

        qCWarning(C_VALIDATORBEFORE) << "Invalid validation data:" << d->date;
        setValidationDataError(true);
        return false;

    }

    return false;
}



QString ValidatorBefore::genericErrorMessage() const
{
    Q_D(const ValidatorBefore);

    switch(d->date.type()) {
    case QVariant::Date:
        return tr("The date in the “%1” field must be before %2.").arg(genericFieldName(),
                                                                      //: date shown in validator error message
                                                                      d->date.toDate().toString(tr("MM/dd/yyyy")));
    case QVariant::DateTime:
        return tr("The date and time in the “%1” field must be before %2.").arg(genericFieldName(),
                                                                               //: date and time shown in validator error message
                                                                               d->date.toDateTime().toString(tr("MM/dd/yyyy hh:mm")));
    case QVariant::Time:
        return tr("The time in the “%1” field must be before %2.").arg(genericFieldName(),
                                                                      //: time shown in the validator error message
                                                                      d->date.toTime().toString(tr("hh:mm")));
    default:
        return QString();
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
