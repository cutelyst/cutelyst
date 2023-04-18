/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORJSON_H
#define CUTELYSTVALIDATORJSON_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorJsonPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorJson validatorjson.h <Cutelyst/Plugins/Utils/validatorjson.h>
 * \brief Checks if the inut data is valid JSON.
 *
 * This tries to load the input \a field string into a QJsonDocument and checks if it is not null and not empty.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorJson : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new json validator.
     * \param field         Name of the input field to validate.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorJson(const QString &field, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the json validator.
     */
    ~ValidatorJson() override;

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value converted into a QJsonDocument.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     * \param c         The current context, used for translations.
     * \param errorData Will contain the error string from QJsonParseError.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorJson)
    Q_DISABLE_COPY(ValidatorJson)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORJSON_H
