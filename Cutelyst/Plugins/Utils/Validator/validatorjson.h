/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CUTELYSTVALIDATORJSON_H
#define CUTELYSTVALIDATORJSON_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorJsonPrivate;

/*!
 * \brief Checks if the inut data is valid JSON.
 *
 * This tries to load the input \a field string into a QJsonDocument and checks if it is not null and not empty.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorJson : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new json validator.
     * \param field         Name of the input field to validate.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorJson(const QString &field, const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the json validator.
     */
    ~ValidatorJson();
    
    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError() const override;
    
    /*!
     * Constructs a new ValidatorJson object with the given private class.
     */
    ValidatorJson(ValidatorJsonPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorJson)
    Q_DISABLE_COPY(ValidatorJson)
};
    
}

#endif //CUTELYSTVALIDATORJSON_H

