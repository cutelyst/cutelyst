/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORAFTER_H
#define CUTELYSTVALIDATORAFTER_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorAfterPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorafter.h>
 * \brief Checks if a date, time or datetime is after a comparison value.
 *
 * This will check if the date, time or datetime in the input \a field is earlier than the \a
 * comparison value set in the constructor. It depends on the comparison value how the input data is
 * handled. If the comparison value is a QDateTime, the input data will be converted into a
 * QDateTime to compare the both values. The same happens for QTime and QDate. It is also possible
 * to use a QString as \a comparison value. Using a QString makes it possible to compare the input
 * field against a value from the \link Context::stash() stash\endlink.
 *
 * To specify a time zone that should be used for the input field - and the comparison input field
 * if one is used, give either the IANA time zone ID name to the \a timeZone argument of the
 * constructor or the name of an input field or stash key that contains the ID name. It will then be
 * first tried to create a valid QTimeZone from the \a timeZone string, if that fails it will first
 * try to get the time zone from the input parameters and if there is no key with that name, trying
 * it with the \link Context::stash() stash\endlink. Stash or input parameter can either contain a
 * valid IANA time zone ID or the offset from UTC in seconds.
 *
 * If the input data can not be parsed into a comparable format, the validation fails and
 * ValidatorRule::parsingError() will return the error string. It will also fail if the comparison
 * value is not of type QDate, QTime, QDateTime or QString. Use \a inputFormat parameter of the
 * constructor to set a custom input format. If that fails or if there is no custom \a inputFormat
 * set, it will try to parse the date based on standard formats in the following order: \link
 * Context::locale() Context locale's \endlink \link QLocale::toDate() toDate() \endlink with
 * QLocale::ShortFormat and QLocale::LongFormat, Qt::ISODate, Qt::RFC2822Date and Qt::TextDate
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Examples
 * \code{.cpp}
 * void MyController::form_do(Context *c)
 * {
 *     Validator v({
 *                  // compare against a specific date
 *                  new ValidatorAfter(QStringLiteral("datefield"), QDate(2018, 1, 1)),
 *
 *                  // compare against a specific date and time that might have a format different
 *                  // for every language
 *                  new ValidatorAfter(QStringLiteral("datefield2"),
 *                                     QDateTime::fromString(QStringLiteral("2018-01-01T18:18:18"),
 *                                                           Qt::ISODate),
 *                                     {},
 *                                     QT_TRANSLATE_NOOP("MyController", "MM/dd/yyyy, HH:mm")),
 *
 *                  // compare against a datetime in the stash
 *                  new ValidatorAfter(QStringLiteral("datetime"), QStringLiteral("stashkey")),
 *
 *                  // compare against a datetime and set the input fields time zone
 *                  new ValidatorAfter(QStringLiteral("datetime2"),
 *                                     QDateTime(QDate(2018, 1, 15),
 *                                               QTime(12,0),
 *                                               QTimeZone(QByteArrayLiteral("Europe/Berlin")),
 *                                     QStringLiteral("America/Rio_Branco")),
 *
 *                  // compare against a datetime and read input fields time zone from another
 *                  // input field
 *                  new ValidatorAfter(QStringLiteral("datetime3"),
 *                                     QDateTime(QDate(2018, 1, 15),
 *                                               QTime(12,0),
 *                                               QTimeZone(QByteArrayLiteral("Europe/Berlin")),
 *                                     QStringLiteral("tz_field"))
 *                }, QLatin1String("MyController"));
 * }
 * \endcode
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QDateTime, QDate or QTime, according
 * to the type of the comparison value.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorBefore
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAfter : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorAfter object with the fiven parameters.
     *
     * \param field         Name of the input field to validate.
     * \param comparison    The value or stash key to compare against.
     * \param timeZone      IANA time zone ID or stash key containing the ID
     * \param inputFormat   Optional input format for input data parsing, can be translatable.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    ValidatorAfter(const QString &field,
                   const QVariant &comparison,
                   const QString &timeZone           = {},
                   const char *inputFormat           = nullptr,
                   const ValidatorMessages &messages = ValidatorMessages(),
                   const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorAfter object.
     */
    ~ValidatorAfter() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeds, ValidatorReturnType::value will contain the converted input
     * parameter value as QDateTime, QDate or QTime, accoring to the type of the \a comparison
     * value.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeds, ValidatorReturnType::value will contain the converted input
     * parameter value as QDateTime, QDate or QTime, accoring to the type of the \a comparison
     * value.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

    /**
     * Returns a generic error if comparison data was invalid.
     */
    QString genericValidationDataError(Context *c,
                                       const QVariant &errorData = QVariant()) const override;

    /**
     * Returns a generic error if the input value could not be parsed.
     */
    QString genericParsingError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorAfter) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorAfter)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORAFTER_H
