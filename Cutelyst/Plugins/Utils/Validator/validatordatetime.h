/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDATETIME_H
#define CUTELYSTVALIDATORDATETIME_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorDateTimePrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatordatetime.h>
 * \brief Checks if the input data is a valid datetime.
 *
 * This validator checks if the input \a field can be parsed into a QDateTime, it will check the
 * parsing ability and will convert the input data into a QDateTime. If a custom \a inputFormat is
 * given, the validator will at first try to parse the datetime according to that format. If that
 * fails or if there is no custom \a inputFormat set, it will try to parse the date and time based
 * on standard formats in the following order: \link Context::locale() Context locale's \endlink
 * \link QLocale::toDate() toDateTime() \endlink with QLocale::ShortFormat and QLocale::LongFormat,
 * Qt::ISODate, Qt::RFC2822Date, Qt::TextDate
 *
 * To specify a time zone that should be used for the input field - and the comparison input field
 * if one is used, give either the IANA time zone ID name to the \a timeZone argument of the
 * constructor or the name of an input field or stash key that contains the ID name. It will then
 * be first tried to create a valid QTimeZone from the \a timeZone, if that fails it will first try
 * to get the time zone from the input parameters and if there is no key with the name trying it
 * with the \link Context::stash() stash\endlink. Stash or input parameter can either contain a
 * valid IANA time zone ID or the offset from UTC in seconds.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QDateTime.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorDate, ValidatorTime
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDateTime : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorDataTime object with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param timeZone      IANA time zone ID, name of a input field containing the ID or stash
     *                      key containing the ID.
     * \param inputFormat   Optional input format for input data parsing, can be translatable.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    explicit ValidatorDateTime(const QString &field,
                               const QString &timeZone,
                               const char *inputFormat           = nullptr,
                               const ValidatorMessages &messages = {},
                               const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorDateTime object.
     */
    ~ValidatorDateTime() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value
     * converted into a QDateTime.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value
     * converted into a QDateTime.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * \brief Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorDateTime) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorDateTime)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDATETIME_H
