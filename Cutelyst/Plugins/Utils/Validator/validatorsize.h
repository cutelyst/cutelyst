/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORSIZE_H
#define CUTELYSTVALIDATORSIZE_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorSizePrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorSize validatorsize.h <Cutelyst/Plugins/Utils/validatorsize.h>
 * \brief The field under validation must have a size matching the given value.
 *
 * Checks if the size of the value in the input \a field is the same as the given \a size.
 *
 * This works for floating point, integer and QString types, where for numeric types it will check the value itself while for QString it will check the string length.
 * Use \a size to define the comparison value and \a type to set the type to check against. \a size will internally converted into a comparative value (qlonglong for QMetaType::Int,
 * qulonglong for QMetaType::UInt and int for QMetaType::QString. Allowed types for the \a type specifier are all numeric types and QMetaType::QString.
 * Any other type will result in a validation data error.
 *
 * If you set a string to the \a size value, this will neither be interpreted as a number nor as string length, but will
 * be used to get the comparison number value from the \link Context::stash() stash\endlink.
 *
 * \note Conversion of numeric input values is performed in the \c 'C' locale.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorMin, ValidatorSize, ValidatorBetween
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorSize : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new size validator.
     * \param field         Name of the input field to validate.
     * \param type          The type to compare.
     * \param size          The size to compare. Will be converted into comparable value. If it is a QString, it will try to get the comparison value from the stash.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorSize(const QString &field, QMetaType::Type type, const QVariant &size, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the size validator.
     */
    ~ValidatorSize() override;

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
     * \brief Returns a generic error message for validation data errors.
     * \param c         The current context, used for translations.
     * \param errorData Will contain either 1 if comparison value is invalid or 0 if the \a type is not supported.
     */
    QString genericValidationDataError(Context *c, const QVariant &errorData) const override;

    /*!
     * \brief Returns a generic error message for input value parsing errors.
     */
    QString genericParsingError(Context *c, const QVariant &errorData) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorSize)
    Q_DISABLE_COPY(ValidatorSize)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORSIZE_H
