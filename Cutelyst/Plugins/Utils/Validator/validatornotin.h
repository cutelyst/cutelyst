/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORNOTIN_H
#define CUTELYSTVALIDATORNOTIN_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

#include <QStringList>

namespace Cutelyst {

class ValidatorNotInPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatornotin.h>
 * \brief Checks if the field value is not one from a list of values.
 *
 * This validator checks if the value of the \a field is not one from a list of \a values.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QString.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorIn
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorNotIn : public ValidatorRule
{
public:
    /**
     * Constructs a new %VaidatorNotIn with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param values        List of values to compare against. Can be either a QStringList
     *                      containing the not allowed values or a QString specifing a stash
     *                      key containing a QStringList with not allowed values.
     * \param cs            Case sensitivity when comparing the values.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    ValidatorNotIn(const QString &field,
                   const QVariant &values,
                   Qt::CaseSensitivity cs            = Qt::CaseSensitive,
                   const ValidatorMessages &messages = ValidatorMessages(),
                   const QString &defValKey          = QString());

    /**
     * Destroys the %ValidatorNotIn object.
     */
    ~ValidatorNotIn() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

    /**
     * Returns a generic error messages if the list of comparison values is empty.
     */
    QString genericValidationDataError(Context *c, const QVariant &errorData) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorNotIn) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorNotIn)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORNOTIN_H
