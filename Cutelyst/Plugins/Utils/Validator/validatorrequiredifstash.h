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
#ifndef CUTELYSTVALIDATORREQUIREDIFSTASH_H
#define CUTELYSTVALIDATORREQUIREDIFSTASH_H

#include "validatorrule.h"
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorRequiredIfStashPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorRequiredIfStash validatorrequiredifstash.h <Cutelyst/Plugins/Utils/validatorrequiredifstash.h>
 * \brief The field under validation must be present and not empty if the content of a stash key is equal to one from a list.
 *
 * If the value of the \link Context::stash() stash\endlink key is equal to one of the values defined in the \a stashValues list,
 * the input \a field under validation must be present and not empty. This validator is the opposite of ValidatorRequiredUnlessStash
 * and it is similar to ValidatorRequiredIf.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation. So, fields that only contain
 * whitespaces will be treated as empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorRequired, ValidatorRequiredIf, ValidatorRequiredUnless, ValidatorRequiredWith, ValidatorRequiredWithAll, ValidatorRequiredWithout,
 * ValidatorRequiredWithoutAll, ValidatorRequiredUnlessStash
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorRequiredIfStash : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new required if stash validator.
     * \param field         Name of the input field to validate.
     * \param stashKey      Name of the stash key to compare against.
     * \param stashValues   Values in the \a stashKey from which one must match the content of the stash key to require the \a field.
     * \param messages      Custom error messages if validation fails.
     */
    ValidatorRequiredIfStash(const QString &field, const QString &stashKey, const QVariantList &stashValues, const ValidatorMessages &messages = ValidatorMessages());

    /*!
     * \brief Deconstructs the required if validator.
     */
    ~ValidatorRequiredIfStash();

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
    Q_DECLARE_PRIVATE(ValidatorRequiredIfStash)
    Q_DISABLE_COPY(ValidatorRequiredIfStash)
};

}

#endif // CUTELYSTVALIDATORREQUIREDIFSTASH_H
