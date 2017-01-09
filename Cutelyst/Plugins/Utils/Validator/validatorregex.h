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
#ifndef CUTELYSTVALIDATORREGEX_H
#define CUTELYSTVALIDATORREGEX_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QObject>
#include <QRegularExpression>

namespace Cutelyst {
    
class ValidatorRegexPrivate;

/*!
 * \brief The field under validation must match the given regular expression.
 *
 * Checks if the \a regex matches the content of the \a field.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRegex : public ValidatorRule
{
    Q_OBJECT
public:
    /*!
     * \brief Constructs a new regex validator.
     * \param field         Name of the input field to validate.
     * \param regex         The regular expression to check against.
     * \param label         Human readable input field label, used for generic error messages.
     * \param customError   Custom errror message if validation fails.
     * \param parent        Parent object.
     */
    ValidatorRegex(const QString &field, const QRegularExpression &regex, const QString &label = QString(), const QString &customError = QString(), QObject *parent = nullptr);
    
    /*!
     * \brief Deconstructs the regex validator.
     */
    ~ValidatorRegex();
    
    /*!
     * \brief Performs the validation.
     *
     * Returns \c true on success.
     */
    bool validate() override;

    /*!
     * \brief Sets the regular expression to check.
     */
    void setRegex(const QRegularExpression &regex);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericErrorMessage() const override;
    
    ValidatorRegex(ValidatorRegexPrivate &dd, QObject *parent);
    
private:
    Q_DECLARE_PRIVATE(ValidatorRegex)
    Q_DISABLE_COPY(ValidatorRegex)
};
    
}

#endif //CUTELYSTVALIDATORREGEX_H

