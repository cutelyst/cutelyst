/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORFILESIZE_H
#define CUTELYSTVALIDATORFILESIZE_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QLocale>

namespace Cutelyst {

class ValidatorFileSizePrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorFileSize validatorfilesize.h <Cutelyst/Plugins/Utils/validatorfilesize.h>
 * \brief Checks if the input field contains a valid file size string like 1.5 GB.
 *
 * The \a field under validation is ownly allowed to contain a size number and a prefix symbol for
 * either binary (MiB, etc.) or decimal (MB, etc.) file sizes. Negative values are not allowed. White
 * space (space and tab) is allowed and will be ignored by the validation logic. Size prefixes from kB
 * to YiB are supported.
 *
 * This validation rule is locale aware, so be sure to set the correct \link Context::locale() locale
 * on your context\endlink. The locale awareness is especially important to select the correct decimal separator
 * sign. To support both language directions, the validation logic supports both positions of the prefix symbol
 * for all languages (e.g. "KB 2" and "2 KB" are handled the same way and are both valid).
 *
 * Unless a specific \link ValidatorFileSize::Option option\endlink has been selected, the validator will use binary
 * or decimal base according to the prefix. (2KiB will return 2048, 2kB will return 2000). Recognizion of the
 * prefix is case insensitive.
 *
 * \note The conversion of big sizes like Petabyte and above might lead to rounding issues in the return value.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * If you want to use a <a href="https://developer.mozilla.org/en-US/docs/Web/HTML/Element/input#attr-pattern">pattern</a>
 * in your HTML input element that matches this validator, use ValidatorFileSize::inputPattern().
 *
 * \sa Validator for general usage of validators.
 *
 * \since Cutelyst 2.0.0
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorFileSize : public ValidatorRule
{
public:
    /*!
     * \brief Options for %ValidatorFileSize
     */
    enum Option : quint8 {
        NoOption     = 0, /**< No option to use. */
        OnlyBinary   = 1, /**< Accepts only binary prefix symbols like KiB, MiB, etc. */
        OnlyDecimal  = 2, /**< Accepts only decimal prefix symbols like kB, MB, etc. */
        ForceBinary  = 3, /**< Forces the usage of the binary system when generating the return value but will not fail if the prefix symbol is decimal. */
        ForceDecimal = 4  /**< Forces the usage of the decimal system when generating the return value but will not fail if the prefix symbol is binray. */
    };

    /*!
     * \brief Constructs a new file size validator.
     * \param field     Name of the input field to validate.
     * \param option    Option to use when validating and generating the return value.
     * \param min       Optional minimum size, can be either a number value to compare against or the name of a \link Context::stash()
     * stash\endlink key that contains the value. To disable, use a value lower \c 0 or an invalid QVariant.
     * \param max       Optional maximum size, can be either a number value to compare against or the name of a \link Context::stash()
     * stash\endlink key that contains the value. To disable, use a value lower \c 0 or an invalid QVariant.
     * \param messages  Custom error messages if validation fails.
     * \param defValKey \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorFileSize(const QString &field,
                      Option option                     = NoOption,
                      const QVariant &min               = QVariant(),
                      const QVariant &max               = QVariant(),
                      const ValidatorMessages &messages = ValidatorMessages(),
                      const QString &defValKey          = QString());

    /*!
     * \brief Deconstructs the file size validator.
     */
    ~ValidatorFileSize() override;

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a value is a valid file size string.
     * \param[in] value     The value to validate.
     * \param[in] min       Optional minimum size. Use a number lower \c 0 to disable the check.
     * \param[in] max       Optional maximum size. Use a number lower \c 0 to disable the check.
     * \param[in] option    \link ValidatorFileSize::Option Option\endlink to use when validating and generating the \a fileSize.
     * \param[in] locale    The locale to use when validating the input value.
     * \param[out] fileSize Optional pointer to a double variable that will contain the extracted file size if validation succeeded.
     * \return \c true if \a value is a valid file size string.
     * \since Cutelyst 2.0.0
     */
    static bool validate(const QString &value,
                         double min            = -1,
                         double max            = -1,
                         Option option         = NoOption,
                         const QLocale &locale = QLocale(),
                         double *fileSize      = nullptr);

    /*!
     * \brief Puts an HTML input pattern for file sizes into the stash.
     *
     * This will either put \c "^\\d+[,.٫]?\\d*\\s*[KkMmGgTt]?[Ii]?[Bb]?" into the \a stashKey if the \link Context::locale()
     * current locale's\endlink direction is from left to right, or \c "[KkMmGgTt]?[Ii]?[Bb]?\\s*\\d+[,.٫]?\\d*" if the
     * direction is right to left.
     *
     * \param c         Pointer to the current context.
     * \param stashKey  Name of the stash key to put the pattern in.
     */
    static void inputPattern(Context *c, const QString &stashKey = QStringLiteral("fileSizePattern"));

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value either as qulonglong or double.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

    /*!
     * \brief Returns a generic error messages if validation data is missing or invalid.
     *
     * \a errorData will contain either \c 0 if the minimum size value is invalid, or \c 1 if the
     * maximum size value is invalid.
     */
    QString genericValidationDataError(Context *c, const QVariant &errorData) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorFileSize)
    Q_DISABLE_COPY(ValidatorFileSize)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORFILESIZE_H
