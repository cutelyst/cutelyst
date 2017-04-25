/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef CUTELYSTVALIDATORACCEPTED_H
#define CUTELYSTVALIDATORACCEPTED_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorAcceptedPrivate;

/*!
 * \brief Checks if a field is available and has a specific value.
 *
 * The \a field under validation must be \c yes, \c on, \c 1, or \c true. This is useful for validating "Terms of Service" acceptance.
 * This check will also fail if the input data for the specified \a field is empty or if the \a field is not part of the input data.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAccepted : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new accepted validator.
     *
     * \param field         Name of the input field to validate.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorAccepted(const QString &field, const QString &label = QString(), const QString &customError = QString());

    /*!
     * \brief Deconstructs the accepted validator.
     */
    ~ValidatorAccepted();

    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

protected:
    /*!
     * \brief Creates a generic error message.
     */
    QString genericValidationError() const override;

    /*!
     * Constructs a new ValidatorAccepted object with the given private class.
     */
    ValidatorAccepted(ValidatorAcceptedPrivate &dd);

private:
    Q_DECLARE_PRIVATE(ValidatorAccepted)
    Q_DISABLE_COPY(ValidatorAccepted)
};

}


#endif //CUTELYSTVALIDATORACCEPTED_H
