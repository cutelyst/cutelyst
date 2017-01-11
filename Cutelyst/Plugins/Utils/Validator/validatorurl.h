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
#ifndef CUTELYSTVALIDATORURL_H
#define CUTELYSTVALIDATORURL_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorUrlPrivate;

/*!
 * \brief The field under validation must be a valid URL.
 *
 * Checks if the \a field contains a valid URL by loading it into a QUrl and testing it's validity.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorUrl : public ValidatorRule
{
public:
    /*!
     * \brief Constraints to limit the validation.
     */
    enum Constraint {
        NoConstraint    = 0,    /**< No constraints set. */
        StrictParsing   = 1,    /**< String will be parsed in strict mode. See http://doc.qt.io/qt-5/qurl.html#ParsingMode-enum */
        NoRelative      = 2,    /**< Relative URLs are not valid. */
        NoLocalFile     = 4,    /**< Local file URLs are not Vaid. */
        WebsiteOnly     = 8,    /**< Combines NoRelative and NoLocalFile and sets http and https to the schemes list. (Will overwrite existing list) */
    };
    Q_DECLARE_FLAGS(Constraints, Constraint)

    /*!
     * \brief Constructs a new url validator.
     * \param field         Name of the input field to validate.
     * \param constraints   Constraints for parsing and validating the URL.
     * \param schemes       List of allowed schemes for a valid URL.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom error message if validation fails.
     */
    ValidatorUrl(const QString &field, Constraints constraints = NoConstraint, const QStringList &schemes = QStringList(), const QString &label = QString(), const QString &customError = QString());
    
    /*!
     * \brief Deconstructs the validator.
     */
    ~ValidatorUrl();
    
    /*!
     * \brief Performs the validation and returns an empty QString on success, otherwise an error message.
     */
    QString validate() const override;

    /*!
     * \brief Sets optional validation contraints.
     */
    void setConstraints(Constraints constraints);

    /*!
     * \brief Sets an optional list of valid schemes.
     */
    void setSchemes(const QStringList &schemes);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericValidationError() const override;
    
    ValidatorUrl(ValidatorUrlPrivate &dd);
    
private:
    Q_DECLARE_PRIVATE(ValidatorUrl)
    Q_DISABLE_COPY(ValidatorUrl)
};
    
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::ValidatorUrl::Constraints)

#endif //CUTELYSTVALIDATORURL_H

