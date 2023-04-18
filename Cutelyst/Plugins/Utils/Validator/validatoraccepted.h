/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORACCEPTED_H
#define CUTELYSTVALIDATORACCEPTED_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorAcceptedPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorAccepted validatoraccepted.h <Cutelyst/Plugins/Utils/validatoraccepted.h>
 * \brief Checks if a field is available and has a specific value.
 *
 * The \a field under validation must be \c yes, \c on, \c 1, or \c true. This is useful for validating "Terms of Service" acceptance.
 * This check will also fail if the input data for the specified \a field is empty or if the \a field is not part of the input data.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAccepted : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new accepted validator.
     *
     * \param field     Name of the input field to validate.
     * \param messages  Custom error message if validation fails.
     */
    ValidatorAccepted(const QString &field, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the accepted validator.
     */
    ~ValidatorAccepted() override;

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if the \a value is \c yes, \c on, \c 1, or \c true.
     * \param value The value to validate.
     * \return \c true if the \a value is \c yes, \c on, \c 1, or \c true.
     */
    static bool validate(const QString &value);

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain \c true.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Creates a generic error message.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorAccepted)
    Q_DISABLE_COPY(ValidatorAccepted)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORACCEPTED_H
