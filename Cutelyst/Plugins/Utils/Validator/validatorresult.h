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
#ifndef CUTELYSTVALIDATORRESULT_H
#define CUTELYSTVALIDATORRESULT_H

#include <Cutelyst/cutelyst_global.h>
#include <QString>
#include <QStringList>
#include <QSharedDataPointer>

namespace Cutelyst {

class ValidatorResultPrivate;

/*!
 * \brief Contains the result of Validator.
 *
 * ValidatorResult will be returned by Validator when calling Validator::validate(). It contains information
 * about occured validation errors, like the error strings of each failed validator and a list of fields where
 * validation failed.
 *
 * Beside the isValid() function, that returns \c true if the complete validation process was successful and \c false
 * if any of the validators failed, it provides a bool operator that makes it usable in \c if statements.
 *
 * \code{.cpp}
 * static Validator v(c, {new ValidatorRequired(QStringLiteral("required_field")});
 * ValidatorResult r = v.validate();
 * if (r) {
 *      // do stuff if validation was successful
 * } else {
 *      // do other stuff if validation failed
 * }
 *
 *
 * // you can also comapre the boolen value of the result directly
 * // for example together with the Validator::FillStashOnError flag
 * // to automatically fill the stash with error information and input values
 * if (v.validate(Validator::FillStashOnError)) {
 *      // do stuff if validation was successful
 * }
 * \endcode
 *
 * Validity is simply determined by the fact, that it does not contain any error information.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorResult {
public:
    /*!
     * \brief Constructs a new ValidatorResult.
     *
     * A newly constructed ValidatorResult willl be valid by default, because it
     * does not cotain any error information.
     */
    ValidatorResult();

    /*!
     * \brief Creates a copy of \a other.
     */
    ValidatorResult(const ValidatorResult &other);

    /*!
     * \brief Deconstructs the ValidatorResult.
     */
    ~ValidatorResult();

    /*!
     * \brief Returns \c true if the validation was successful.
     *
     * \note A newly constructed ValidatorResult will be valid by default.
     */
    bool isValid() const;

    /*!
     * \brief Adds new error information to the
     * \param field     Name of the input field that has input errors.
     * \param message   Error message shown to the user.
     */
    void addError(const QString &field, const QString &message);

    /*!
     * \brief Returns a list of error messages.
     *
     * Returns a list of all error messages from every failed ValidatorRule.
     */
    QStringList errorStrings() const;

    /*!
     * \brief Returns a list of field names with errors.
     *
     * Returns a list of all field names for that a ValidatorRule failed.
     */
    QStringList errorFields() const;

    /*!
     * \brief Returns \c true if the validation was successful.
     *
     * \note A newly constructed ValidatorResult will be valid by default.
     */
    explicit operator bool() const {
        return isValid();
    }

private:
    QSharedDataPointer<ValidatorResultPrivate> d;
};

}

#endif // CUTELYSTVALIDATORRESULT_H
