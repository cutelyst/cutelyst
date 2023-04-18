/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORPWQUALITY_H
#define CUTELYSTVALIDATORPWQUALITY_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorPwQualityPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorPwQuality validatorpwquality.h <Cutelyst/Plugins/Utils/validatorpwquality.h>
 * \brief Validates an input field with libpwquality to check password quality.
 *
 * This validator uses <a href="https://github.com/libpwquality/libpwquality">libpwquality</a> to generate a
 * password quality score that is compared against a \a threshold. If it is below the \a threshold, the validation
 * fails. According to libpwquality a score of 0-30 is of low, a score of 30-60 of medium and a score of 60-100
 * of high quality. Everything below 0 is an error and the password should not be used.
 *
 * <h3>Building</h3>
 * As this validator relies on an external library, it will not be included and build by default. Use either
 * <code>-DPLUGIN_VALIDATOR_PWQUALITY:BOOL=ON</code> or <code>-DBUILD_ALL:BOOL=ON</code> when configuring %Cutelyst
 * for build with cmake. In your %Cutelyst application you can check if \c CUTELYST_VALIDATOR_WITH_PWQUALITY has been
 * defined to see if this validator is available.
 *
 * <h3>Options</h3>
 * %ValidatorPwQuality can take additional \a options. To learn more about the available options see <code>man 5 pwquality.conf</code>.
 * The options value can be either a QVariantMap containing the options or a QString specifying a file name that will be read
 * by libpwquality. For the constructor the options will also be searched in the current \link Context::stash() stash\endlink if
 * it is a QString. The stash value should than be either a QVariantMap or a QString pointing to a configuration file. All values
 * in the QVariantMap used to specify \a options, have to be convertible into QString. The QVariantMap does not have to contain
 * all available option keys, for keys that are not contained, the default values of libpwquality will be used. If the \a options
 * QVariant is not valid or if a contained QString or QVariantMap is empty, the options from the default libpwquality configuration
 * file will be read.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \since Cutelyst 2.0.0
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorPwQuality : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new %ValidatorPwQuality with the given parameters.
     * \param field     Name of the input field to validate.
     * \param threshold The quality score threshold below the validation fails.
     * \param options   Options for libpwquality. Use invalid QVariant to omit.
     * \param userName  Input params key or stash key containing the user name, used for quality checks.
     * Will first try params, than stash. Leave empty to omit.
     * \param oldPassword   Input params key or stash key containing the old password, used for quality checks.
     * Will first try params, than stash. Leave empty to omit.
     * \param messages  Custom error messages if validation fails.
     */
    explicit ValidatorPwQuality(const QString &field,
                                int threshold                     = 30,
                                const QVariant &options           = QVariant(),
                                const QString &userName           = QString(),
                                const QString &oldPassword        = QString(),
                                const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the %ValidatorPwQuality.
     */
    ~ValidatorPwQuality() override;

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns the password quality score for \a value.
     * \param value         The value to validate.
     * \param options       Options for libpwquality.
     * \param oldPassword   Optional old password used for some checks.
     * \param user          Optional user name used for some checks.
     * \return the password quality score, everything below \c 0 is an error, everything >= 0 is a quality score where
     * 0-30 is low, 30-60 medium and 60-100 high quality. You can use ValidatorPwQuality::errorString() to get a human
     * readable string explaining the return value.
     */
    static int validate(const QString &value, const QVariant &options = QVariant(), const QString &oldPassword = QString(), const QString &user = QString());

    /*!
     * \brief Returns a human readable string for the return value of ValidatorPwQuality::validate()
     * \param c             Current context, used for translations.
     * \param returnValue   The return value of ValidatorPwQuality::validate()
     * \param label         Optional label used in the returned string.
     * \param threshold     The threshold below that a password is invalid.
     * \return Human readable error string. If \a returnValue is >= 0 but below \a threshold, a string explaining the
     * threshold will be returned. If \a returnValue is >= \a threshold, an empty string will be returned.
     */
    static QString errorString(Context *c, int returnValue, const QString &label = QString(), int threshold = 0);

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     *
     * The \a errorData will contain the score returned by ValidatorPqQuality::validate()
     */
    QString genericValidationError(Context *c, const QVariant &errorData) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorPwQuality)
    Q_DISABLE_COPY(ValidatorPwQuality)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORPWQUALITY_H
