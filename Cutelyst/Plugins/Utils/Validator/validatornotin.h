/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORNOTIN_H
#define CUTELYSTVALIDATORNOTIN_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QStringList>

namespace Cutelyst {

class ValidatorNotInPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorNotIn validatornotin.h <Cutelyst/Plugins/Utils/validatornotin.h>
 * \brief Checks if the field value is not one from a list of values.
 *
 * This validator checks if the value of the \a field is not one from a list of \a values.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorIn
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorNotIn : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new not in validator.
     * \param field         Name of the input field to validate.
     * \param values        List of values to compare against.
     * \param cs            Case sensitivity when comparing the values.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorNotIn(const QString &field, const QStringList &values, Qt::CaseSensitivity cs = Qt::CaseSensitive, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the validator.
     */
    ~ValidatorNotIn() override;

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
     * \brief Returns a generic error messages if the list of comparison values is empty.
     */
    QString genericValidationDataError(Context *c, const QVariant &errorData) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorNotIn)
    Q_DISABLE_COPY(ValidatorNotIn)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORNOTIN_H
