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

#include "validatorafter_p.h"

#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_VALIDATORAFTER, "cutelyst.utils.validator.after")

ValidatorAfter::ValidatorAfter(const QString &field, const QVariant &dateTime, const QString &inputFormat, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorAfterPrivate(field, dateTime, inputFormat, label, customError))
{

}

ValidatorAfter::ValidatorAfter(ValidatorAfterPrivate &dd) :
    ValidatorRule(dd)
{

}

ValidatorAfter::~ValidatorAfter()
{

}

QString ValidatorAfter::validate() const
{
    QString result;

    Q_D(const ValidatorAfter);

    const QString v = value();

    if (!v.isEmpty()) {

        if (d->date.type() == QVariant::Date) {

            QDate odate = d->date.toDate();
            if (!odate.isValid()) {
                qCWarning(C_VALIDATORAFTER) << "Invalid validation date.";
                result = validationDataError();
            } else {
                QDate date = d->extractDate(v, d->inputFormat);
                if (!date.isValid()) {
                    qCWarning(C_VALIDATORAFTER) << "Can not parse input date:" << v;
                    result = parsingError();
                } else {
                    if (date <= odate) {
                        result = validationError();
                    }
                }
            }

        } else if (d->date.type() == QVariant::DateTime) {

            QDateTime odatetime = d->date.toDateTime();
            if (!odatetime.isValid()) {
                qCWarning(C_VALIDATORAFTER) << "Invalid validation date and time.";
                result = validationDataError();
            } else {
                QDateTime datetime = d->extractDateTime(v, d->inputFormat);
                if (!datetime.isValid()) {
                    qCWarning(C_VALIDATORAFTER) << "Can not parse input date and time:" << v;
                    result = parsingError();
                } else {
                    if (datetime <= odatetime) {
                        result = validationError();
                    }
                }
            }

        } else if (d->date.type() == QVariant::Time) {

            QTime otime = d->date.toTime();
            if (!otime.isValid()) {
                qCWarning(C_VALIDATORAFTER) << "Invalid validation time.";
                result = validationDataError();
            } else {
                QTime time = d->extractTime(v, d->inputFormat);
                if (!time.isValid()) {
                    qCWarning(C_VALIDATORAFTER) << "Can not parse input time:" << v;
                    result = parsingError();
                } else {
                    if (time <= otime) {
                        result = validationError();
                    }
                }
            }

        } else {
            qCWarning(C_VALIDATORAFTER) << "Invalid validation data:" << d->date;
            result = validationDataError();
        }
    }

    return result;
}

QString ValidatorAfter::genericValidationError() const
{
    QString error;

    Q_D(const ValidatorAfter);

    switch(d->date.type()) {
    case QVariant::Date:
        error = QStringLiteral("The date in the “%1” field must be after %2.").arg(fieldLabel(),
                                                                                   //: date shown in validator error message
                                                                                   d->date.toDate().toString(QStringLiteral("dd.MM.yyyy")));
        break;
    case QVariant::DateTime:
        error = QStringLiteral("The date and time in the “%1” field must be after %2.").arg(fieldLabel(),
                                                                                            //: date and time shown in validator error message
                                                                                            d->date.toDateTime().toString(QStringLiteral("dd.MM.yyyy HH:mm")));
        break;
    case QVariant::Time:
        error = QStringLiteral("The time in the “%1” field must be after %2.").arg(fieldLabel(),
                                                                                   //: time shown in the validator error message
                                                                                   d->date.toTime().toString(QStringLiteral("dd.MM.yyyy HH:mm")));
        break;
    default:
        error = validationDataError();
        break;
    }

    return error;
}

void ValidatorAfter::setDateTime(const QVariant &dateTime)
{
    Q_D(ValidatorAfter);
    d->date = dateTime;
}

void ValidatorAfter::setInputFormat(const QString &format)
{
    Q_D(ValidatorAfter);
    d->inputFormat = format;
}
