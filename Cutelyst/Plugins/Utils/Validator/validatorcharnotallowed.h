/*
 * SPDX-FileCopyrightText: (C) 2019-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORCHARNOTALLOWED_H
#define CUTELYSTVALIDATORCHARNOTALLOWED_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorCharNotAllowedPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorCharNotAllowed validatorcharnotallowed.h <Cutelyst/Plugins/Utils/validatorcharnotallowed.h>
 * \brief Validates an input field for not allowed characters.
 *
 * The \a field under validation is not allowed to contain a list of characters.
 * The list of not allowed characters is set as a QString to the \a forbiddenChars.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRegularExpression
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorCharNotAllowed : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new char not allowed validator.
     *
     * \param field             Name of the input field to validate.
     * \param forbiddenChars    List of characters not allowed in the input field.
     * \param messages          Custom error messages if validation fails.
     * \param defValKey         \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorCharNotAllowed(const QString &field, const QString &forbiddenChars, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the char not allowed validator.
     */
    ~ValidatorCharNotAllowed() override;

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a value does not contain any of the characters in \a forbiddenChars
     * \param value             The value to validate.
     * \param forbiddenChars    The list of forbidden characters.
     * \param foundChar         If set, it will contain the first found chararacter that is forbidden.
     * \return \c true if the \a value does not contain any of the \a forbiddenChars
     */
    static bool validate(const QString &value, const QString &forbiddenChars, QChar *foundChar = nullptr);

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
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

    /*!
     * \brief Returns a generic error if the list of forbidden characters is empty.
     */
    QString genericValidationDataError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorCharNotAllowed)
    Q_DISABLE_COPY(ValidatorCharNotAllowed)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORCHARNOTALLOWED_H
