/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORJSON_H
#define CUTELYSTVALIDATORJSON_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorJsonPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorjson.h>
 * \brief Checks if the inut data is valid JSON.
 *
 * This tries to load the input \a field string into a QJsonDocument and checks if it is not null
 * and not empty and if it complies to the expected type.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QJsonDocument if \a expectedType is set to
 * ExpectedType::All, a QJsonArray if \a expectedType is set to ExpectedType::Array, or a
 * QJsonObject if \a expectedType is set to ExpectedType::Object.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorJson : public ValidatorRule
{
public:
    /**
     * Defines the expected JSON root type that will be accepted as valid.
     */
    enum class ExpectedType {
        All,   /**< Accepts both, object and array, as valid. If valid, ValidatorReturnType::value
                  will contain a QJsonDocument. */
        Array, /**< Only accepts the input as valid if the root is of type array. If valid,
                  ValidatorReturnType::value will contain a QJsonArray. */
        Object /**< Only accepts the input as valid if the root is of type object. If valid,
                  ValidatorReturnType::value will contain a QJsonObject. */
    };

    /**
     * Constructs a new %ValidatorJson object with the given parameters.
     * \param field         Name of the input field to validate.
     * \param expectedType  Expected JSON root type.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if
     *                      input field is empty. This value will \b NOT be validated.
     */
    ValidatorJson(const QString &field,
                  ExpectedType expectedType         = ExpectedType::All,
                  const ValidatorMessages &messages = ValidatorMessages(),
                  const QString &defValKey          = QString());

    /**
     * Destroys the %ValidatorJson object.
     */
    ~ValidatorJson() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain a QJsonDocument if
     * \a expectedType is set to ExpectedType::All, a QJsonArray if \a expectedType is set
     * to ExpectedType::Array, or a QJsonObject if \a expectedType is set to ExpectedType::Object.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorJson) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorJson)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORJSON_H
