/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORALPHADASH_H
#define CUTELYSTVALIDATORALPHADASH_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorAlphaDashPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorAlphaDash validatoralphadash.h <Cutelyst/Plugins/Utils/validatoralphadash.h>
 * \brief Checks a value for only alpha-numeric content and dashes and underscores.
 *
 * The \a field under validation is only allowed to contain alpha-numeric characters as well as dashes and underscores.
 * If \a asciiOnly is set to \c true, only US-ASCII characters are allowed, otherwise all UTF-8 alpha-numeric characters are allowed.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \par Examples
 * \code
 * "Hallo Kuddel!" // invalid
 * "Hallo_Köddel-2" // valid if asciiOnly is false
 * "Hallo_Kuddel-2" // valid if asciiOnly is true
 * " " // valid if trimBefore is true, invalid if trimBefore is false
 * \endcode
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorAlpha, ValidatorAlphaNum
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAlphaDash : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new alpha dash validator.
     * \param field     Name of the input field to validate.
     * \param asciiOnly If \c true, only ASCII characters are allowed.
     * \param messages  Custom error message if validation fails.
     * \param defValKey \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorAlphaDash(const QString &field, bool asciiOnly = false, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the alpha dash validator.
     */
    ~ValidatorAlphaDash() override;

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if the \a value only contains alpha-numeric characters, dashes and underscores.
     * \param value     The value to validate as it is.
     * \param asciiOnly If \c true, only ASCII characters are allowed.
     * \return \c true if the \a value only contains alpha-numeric characters, dashes and underscores
     */
    static bool validate(const QString &value, bool asciiOnly = false);

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorAlphaDash)
    Q_DISABLE_COPY(ValidatorAlphaDash)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORALPHADASH_H
