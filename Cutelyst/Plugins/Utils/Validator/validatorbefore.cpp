/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorbefore_p.h"

#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_VALIDATORBEFORE, "cutelyst.utils.validator.before", QtWarningMsg)

ValidatorBefore::ValidatorBefore(const QString &field,
                                 const QVariant &comparison,
                                 const QString &timeZone,
                                 const char *inputFormat,
                                 const ValidatorMessages &messages,
                                 const QString &defValKey)
    : ValidatorRule(*new ValidatorBeforePrivate(field,
                                                comparison,
                                                timeZone,
                                                inputFormat,
                                                messages,
                                                defValKey))
{
}

ValidatorBefore::~ValidatorBefore() = default;

ValidatorReturnType ValidatorBefore::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorBefore);

    const QString v = value(params);

    if (!v.isEmpty()) {

        const QTimeZone tz = d->extractTimeZone(c, params, d->timeZone);

        const QVariant _comp =
            (d->comparison.userType() == QMetaType::QString)
                ? d->extractOtherDateTime(c, params, d->comparison.toString(), tz, d->inputFormat)
                : d->comparison;

        if (_comp.userType() == QMetaType::QDate) {

            const QDate odate = _comp.toDate();
            if (Q_UNLIKELY(!odate.isValid())) {
                qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid comparison date";
                result.errorMessage = validationDataError(c);
            } else {
                const QDate date = d->extractDate(c, v, d->inputFormat);
                if (Q_UNLIKELY(!date.isValid())) {
                    qCWarning(C_VALIDATOR).noquote().nospace()
                        << debugString(c) << " Can not parse input date \"" << v << "\"";
                    result.errorMessage = parsingError(c, odate);
                } else {
                    if (Q_UNLIKELY(date >= odate)) {
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << "Input" << date << "is not before" << odate;
                        result.errorMessage = validationError(c, odate);
                    } else {
                        result.value.setValue(date);
                    }
                }
            }

        } else if (_comp.userType() == QMetaType::QDateTime) {

            const QDateTime odatetime = _comp.toDateTime();
            if (Q_UNLIKELY(!odatetime.isValid())) {
                qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid comparison datetime";
                result.errorMessage = validationDataError(c);
            } else {
                const QDateTime datetime = d->extractDateTime(c, v, d->inputFormat, tz);
                if (Q_UNLIKELY(!datetime.isValid())) {
                    qCWarning(C_VALIDATOR).noquote().nospace()
                        << debugString(c) << " Can not parse input datetime \"" << v << "\"";
                    result.errorMessage = parsingError(c, odatetime);
                } else {
                    if (Q_UNLIKELY(datetime >= odatetime)) {
                        qCDebug(C_VALIDATOR).noquote() << debugString(c) << "Input" << datetime
                                                       << "is not before" << odatetime;
                        result.errorMessage = validationError(c, odatetime);
                    } else {
                        result.value.setValue(datetime);
                    }
                }
            }

        } else if (_comp.userType() == QMetaType::QTime) {

            const QTime otime = _comp.toTime();
            if (Q_UNLIKELY(!otime.isValid())) {
                qCWarning(C_VALIDATOR).noquote() << debugString(c) << "Invalid comparison time";
                result.errorMessage = validationDataError(c);
            } else {
                const QTime time = d->extractTime(c, v, d->inputFormat);
                if (Q_UNLIKELY(!time.isValid())) {
                    qCWarning(C_VALIDATOR).noquote().nospace()
                        << debugString(c) << " Can not parse input time \"" << v << "\"";
                    result.errorMessage = parsingError(c, otime);
                } else {
                    if (Q_UNLIKELY(time >= otime)) {
                        qCDebug(C_VALIDATOR).noquote()
                            << debugString(c) << "Input" << time << "is not before" << otime;
                        result.errorMessage = validationError(c, otime);
                    } else {
                        result.value.setValue(time);
                    }
                }
            }

        } else {
            qCWarning(C_VALIDATOR).noquote()
                << debugString(c) << "Invalid comparison data:" << d->comparison;
            result.errorMessage = validationDataError(c);
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorBefore::genericValidationError(Cutelyst::Context *c,
                                                const QVariant &errorData) const
{
    const QString _label = label(c);
    if (_label.isEmpty()) {

        switch (errorData.userType()) {
        case QMetaType::QDate:
            //: %1 will be replaced by the comparison date in short format
            //% "Has to be before %1."
            return c->qtTrId("cutelyst-valbefore-genvalerr-date")
                .arg(c->locale().toString(errorData.toDate(), QLocale::ShortFormat));
        case QMetaType::QDateTime:
            //: %1 will be replaced by the comparison datetime in short format
            //% "Has to be before %1."
            return c->qtTrId("cutelyst-valbefore-genvalerr-dt")
                .arg(c->locale().toString(errorData.toDateTime(), QLocale::ShortFormat));
        case QMetaType::QTime:
            //: %1 will be replaced by the comparison time in short format
            //% "Has to be before %1."
            return c->qtTrId("cutelyst-valbefore-genvalerr-time")
                .arg(c->locale().toString(errorData.toTime(), QLocale::ShortFormat));
        default:
            return validationDataError(c);
        }

    } else {

        switch (errorData.userType()) {
        case QMetaType::QDate:
            //: %1 will be replaced by the field label, %2 by the comparison date in short format
            //% "The date in the “%1” field must be before %2."
            return c->qtTrId("cutelyst-valbefore-genvalerr-date-label")
                .arg(_label, c->locale().toString(errorData.toDate(), QLocale::ShortFormat));
        case QMetaType::QDateTime:
            //: %1 will be replaced by the field label, %2 by the comparison datetime in short
            //: format
            //% "The date and time in the “%1” field must be before %2."
            return c->qtTrId("cutelyst-valbefore-genvalerr-dt-label")
                .arg(_label, c->locale().toString(errorData.toDateTime(), QLocale::ShortFormat));
        case QMetaType::QTime:
            //: %1 will be replaced by the field label, %2 by the comparison time in short format
            //% "The time in the “%1” field must be before %2."
            return c->qtTrId("cutelyst-valbefore-genvalerr-time-label")
                .arg(_label, c->locale().toString(errorData.toTime(), QLocale::ShortFormat));
        default:
            return validationDataError(c);
        }
    }
}

QString ValidatorBefore::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    // the translation strings are defined in ValidatorAfter
    if (_label.isEmpty()) {
        return c->qtTrId("cutelyst-validator-genvaldataerr-dt");
    } else {
        return c->qtTrId("cutelyst-validator-genvaldataerr-dt-label").arg(_label);
    }
}

QString ValidatorBefore::genericParsingError(Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorBefore);

    // translation strings are defined in ValidatorAfter

    const QString _label = label(c);
    if (d->inputFormat) {
        const QString _inputFormatTranslated =
            d->translationContext ? c->translate(d->translationContext, d->inputFormat)
                                  : c->qtTrId(d->inputFormat);
        if (_label.isEmpty()) {
            return c->qtTrId("cutelyst-validator-genparseerr-dt-format")
                .arg(_inputFormatTranslated);
        } else {
            return c->qtTrId("cutelyst-validator-genparseerr-dt-format-label")
                .arg(_label, _inputFormatTranslated);
        }
    } else {

        if (_label.isEmpty()) {
            switch (errorData.userType()) {
            case QMetaType::QDateTime:
                return c->qtTrId("cutelyst-validator-genparseerr-dt");
            case QMetaType::QTime:
                return c->qtTrId("cutelyst-validator-genparseerr-time");
            case QMetaType::QDate:
                return c->qtTrId("cutelyst-validator-genparseerr-date");
            default:
                return validationDataError(c);
            }
        } else {
            switch (errorData.userType()) {
            case QMetaType::QDateTime:
                return c->qtTrId("cutelyst-vaidator-genparseerr-dt-label").arg(_label);
            case QMetaType::QTime:
                return c->qtTrId("cutelyst-validator-genparseerr-time-label").arg(_label);
            case QMetaType::QDate:
                return c->qtTrId("cutelyst-validator-genparseerr-date-label").arg(_label);
            default:
                return validationDataError(c);
            }
        }
    }
}
