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

#include "validatorafter_p.h"

#include <QLocale>
#include <QTimeZone>

using namespace Cutelyst;

ValidatorAfter::ValidatorAfter(const QString &field, const QVariant &comparison, const QString &timeZone, const char *inputFormat, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorAfterPrivate(field, comparison, timeZone, inputFormat, messages, defValKey))
{
}

ValidatorAfter::~ValidatorAfter()
{

}

ValidatorReturnType ValidatorAfter::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorAfter);

    const QString v = value(params);

    if (!v.isEmpty()) {

        const QTimeZone tz = d->extractTimeZone(c, params, d->timeZone);

        const QVariant _comp = (d->comparison.type() == QVariant::String)
                ? d->extractOtherDateTime(c, params, d->comparison.toString(), tz, d->inputFormat)
                : d->comparison;

        if (_comp.type() == QVariant::Date) {

            const QDate odate = _comp.toDate();
            if (Q_UNLIKELY(!odate.isValid())) {
                qCWarning(C_VALIDATOR, "ValidatorAfter: Invalid comparison date and time for field %s at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                result.errorMessage = validationDataError(c);
            } else {
                const QDate date = d->extractDate(c, v, d->inputFormat);
                if (Q_UNLIKELY(!date.isValid())) {
                    qCWarning(C_VALIDATOR, "ValidatorAfter: Can not parse input date \"%s\" for field %s at %s::%s.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                    result.errorMessage = parsingError(c, odate);
                } else {
                    if (Q_UNLIKELY(date <= odate)) {
                        qCDebug(C_VALIDATOR, "ValidatorAfter: Validation failed at %s::%s for field %s: Input date \"%s\" is not after \"%s\".", qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(field()), qPrintable(date.toString()), qPrintable(odate.toString()));
                        result.errorMessage = validationError(c, odate);
                    } else {
                        result.value.setValue<QDate>(date);
                    }
                }
            }

        } else if (_comp.type() == QVariant::DateTime) {

            const QDateTime odatetime = _comp.toDateTime();
            if (Q_UNLIKELY(!odatetime.isValid())) {
                qCWarning(C_VALIDATOR, "ValidatorAfter: Invalid comparison date and time for field %s at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                result.errorMessage = validationDataError(c);
            } else {
                const QDateTime datetime = d->extractDateTime(c, v, d->inputFormat, tz);
                if (Q_UNLIKELY(!datetime.isValid())) {
                    qCWarning(C_VALIDATOR, "ValidatorAfter: Can not parse input date and time \"%s\" for field %s at %s::%s.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                    result.errorMessage = parsingError(c, odatetime);
                } else {
                    if (Q_UNLIKELY(datetime <= odatetime)) {
                        qCDebug(C_VALIDATOR, "ValidatorAfter: Validation failed at %s::%s for field %s: Input date and time \"%s\" is not after \"%s\".", qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(field()), qPrintable(datetime.toString()), qPrintable(odatetime.toString()));
                        result.errorMessage = validationError(c, odatetime);
                    } else {
                        result.value.setValue<QDateTime>(datetime);
                    }
                }
            }

        } else if (_comp.type() == QVariant::Time) {

            const QTime otime = _comp.toTime();
            if (Q_UNLIKELY(!otime.isValid())) {
                qCWarning(C_VALIDATOR, "ValidatorAfter: Invalid comparison time for field %s at %s::%s.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                result.errorMessage = validationDataError(c);
            } else {
                const QTime time = d->extractTime(c, v, d->inputFormat);
                if (Q_UNLIKELY(!time.isValid())) {
                    qCWarning(C_VALIDATOR, "ValidatorAfter: Can not parse input time \"%s\" for field %s at %s::%s.", qPrintable(v), qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
                    result.errorMessage = parsingError(c, otime);
                } else {
                    if (Q_UNLIKELY(time <= otime)) {
                        qCDebug(C_VALIDATOR, "ValidatorAfter: Validation failed at %s::%s for field %s: Input time \"%s\" is not after \"%s\".", qPrintable(c->controllerName()), qPrintable(c->actionName()), qPrintable(field()), qPrintable(time.toString()), qPrintable(otime.toString()));
                        result.errorMessage = validationError(c, otime);
                    } else {
                        result.value.setValue<QTime>(time);
                    }
                }
            }

        } else {
            qCWarning(C_VALIDATOR) << "ValidatorAfter: Invalid validation data for field" << field() << "at" << c->controllerName() << "::" << c->actionName() << ":" << d->comparison;
            result.errorMessage = validationDataError(c);
        }
    } else {
        defaultValue(c, &result, "ValidatorAfter");
    }

    return result;
}

QString ValidatorAfter::genericValidationError(Cutelyst::Context *c, const QVariant &errorData) const
{
    QString error;

    const QString _label = label(c);
    if (_label.isEmpty()) {

        switch (errorData.type()) {
        case QVariant::Date:
            error = QStringLiteral("Has to be after %1.").arg(errorData.toDate().toString(c->locale().dateFormat(QLocale::ShortFormat)));
            break;
        case QVariant::DateTime:
            error = QStringLiteral("Has to be after %1.").arg(errorData.toDateTime().toString(c->locale().dateTimeFormat(QLocale::ShortFormat)));
            break;
        case QVariant::Time:
            error = QStringLiteral("Has to be after %1.").arg(errorData.toTime().toString(c->locale().timeFormat(QLocale::ShortFormat)));
            break;
        default:
            error = validationDataError(c);
            break;
        }

    } else {

        switch(errorData.type()) {
        case QVariant::Date:
            error = c->translate("Cutelyst::ValidatorAfter", "The date in the “%1” field must be after %2.").arg(_label, errorData.toDate().toString(c->locale().dateFormat(QLocale::ShortFormat)));
            break;
        case QVariant::DateTime:
            error = c->translate("Cutelyst::ValidatorAfter", "The date and time in the “%1” field must be after %2.").arg(_label, errorData.toDateTime().toString(c->locale().dateTimeFormat(QLocale::ShortFormat)));
            break;
        case QVariant::Time:
            error = c->translate("Cutelyst::ValidatorAfter", "The time in the “%1” field must be after %2.").arg(_label, errorData.toTime().toString(c->locale().timeFormat(QLocale::ShortFormat)));
            break;
        default:
            error = validationDataError(c);
            break;
        }

    }

    return error;
}

QString ValidatorAfter::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_UNUSED(errorData)
    error = c->translate("Cutelyst::ValidatorAfter", "The comparison value is not a valid date and/or time, or cannot be found.");

    return error;
}

QString ValidatorAfter::genericParsingError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorAfter);

    const QString _label = label(c);
    if (d->inputFormat) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorAfter", "Could not be parsed according to the follwing date and/or time format: %1").arg(c->translate(d->translationContext.data(), d->inputFormat));
        } else {
            error = c->translate("Cutelyst::ValidatorAfter", "The value of the “%1” field could not be parsed according to the follwing date and/or time format: %2").arg(_label, c->translate(d->translationContext.data(), d->inputFormat));
        }
    } else {

        if (_label.isEmpty()) {
            switch (errorData.type()) {
            case QMetaType::QDateTime:
                error = c->translate("Cutelyst::ValidatorAfter", "Could not be parsed as date and time.");
                break;
            case QMetaType::QTime:
                error = c->translate("Cutelyst::ValidatorAfter", "Could not be parsed as time.");
                break;
            case QMetaType::QDate:
                error = c->translate("Cutelyst::ValidatorAfter", "Could not be parsed as date.");
                break;
            default:
                error = validationDataError(c);
                break;
            }
        } else {
            switch (errorData.type()) {
            case QMetaType::QDateTime:
                //: %1 will be replaced by the field label
                error = c->translate("Cutelyst::ValidatorAfter", "The value in the “%1” field could not be parsed as date and time.").arg(_label);
                break;
            case QMetaType::QTime:
                //: %1 will be replaced by the field label
                error = c->translate("Cutelyst::ValidatorAfter", "The value in the “%1” field could not be parsed as time.").arg(_label);
                break;
            case QMetaType::QDate:
                //: %1 will be replaced by the field label
                error = c->translate("Cutelyst::ValidatorAfter", "The value in the “%1” field could not be parsed as date.").arg(_label);
                break;
            default:
                error = validationDataError(c);
                break;
            }
        }
    }

    return error;
}
