/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORTIME_H
#define CUTELYSTVALIDATORTIME_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorTimePrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatortime.h>
 * \brief Checks if the input data is a valid time.
 *
 * This validator checks if the input \a field can be parsed into a QTime, it will check the parsing
 * ability and will convert the input data into a QTime. If a custom \a format is given, the
 * validator will at first try to parse the time according to that \a format. If that fails or if
 * there is no custom \a inputFormat set, it will try to parse the time based on standard formats in
 * the following order: \link Context::locale() Context locale’s \endlink \link QLocale::toDate()
 * toDate() \endlink with QLocale::ShortFormat and QLocale::LongFormat, Qt::ISODate,
 * Qt::RFC2822Date, Qt::TextDate
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QTime.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorDateTime, ValidatorDate
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorTime : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorTime object with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param format        Optional time format for input parsing, can be translatable.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    explicit ValidatorTime(const QString &field,
                           const char *format                = nullptr,
                           const ValidatorMessages &messages = {},
                           const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorTime object.
     */
    ~ValidatorTime() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramater value
     * converted into a QTime.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramater value
     * converted into a QTime.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorTime) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorTime)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORTIME_H
