/*
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef CUTELYSTVALIDATORREQUIREDUNLESSSTASH_H
#define CUTELYSTVALIDATORREQUIREDUNLESSSTASH_H

#include "validatorrule.h"
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorRequiredUnlessStashPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequiredUnlessStash validatorrequiredunlessstash.h <Cutelyst/Plugins/Utils/validatorrequiredunlessstash.h>
 * \brief The \a field under validation must be present and not emptly unless the content of a stash key is equal to a value in a list.
 *
 * If the \link Context::stash() stash\endlink content identified by \a stashKey does \b not contain \b any of the values specified in the
 * \a stashValues list, the \a field under validation must be present and not empty. This validator ist the opposite of ValidatorRequiredIfStash
 * and is similar to ValidatorRequiredUnless.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout,
 * ValidatorRequiredWithoutAll, ValidatorRequiredIfStash
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredUnlessStash : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required unless stash validator.
     * \param field         Name of the input field to validate.
     * \param stashKey      Name of the stash key to compare against.
     * \param stashValues   Values in the \a stashKey from which no one must match the content of the stash key to require the \a field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredUnlessStash(const QString &field, const QString &stashKey, const QVariantList &stashValues, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required unless stash validator.
     */
    ~ValidatorRequiredUnlessStash();

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

private:
    Q_DECLARE_PRIVATE(ValidatorRequiredUnlessStash)
    Q_DISABLE_COPY(ValidatorRequiredUnlessStash)
};

}

#endif // CUTELYSTVALIDATORREQUIREDUNLESSSTASH_H
