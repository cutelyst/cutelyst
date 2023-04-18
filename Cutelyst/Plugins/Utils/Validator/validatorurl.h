/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORURL_H
#define CUTELYSTVALIDATORURL_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorUrlPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorUrl validatorurl.h <Cutelyst/Plugins/Utils/validatorurl.h>
 * \brief The field under validation must be a valid URL.
 *
 * Checks if the \a field contains a valid URL by loading it into a QUrl and testing it's validity.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorUrl : public ValidatorRule
{
public:
    /*!
     * \brief Constraints to limit the validation.
     */
    enum Constraint {
        NoConstraint  = 0, /**< No constraints set. */
        StrictParsing = 1, /**< String will be parsed in strict mode. See http://doc.qt.io/qt-5/qurl.html#ParsingMode-enum */
        NoRelative    = 2, /**< Relative URLs are not valid. */
        NoLocalFile   = 4, /**< Local file URLs are not Vaid. */
        WebsiteOnly   = 8, /**< Combines NoRelative and NoLocalFile and sets http and https to the schemes list. (Will overwrite existing list) */
    };
    Q_DECLARE_FLAGS(Constraints, Constraint)

    /*!
     * \brief Constructs a new url validator.
     * \param field         Name of the input field to validate.
     * \param constraints   Constraints for parsing and validating the URL.
     * \param schemes       List of allowed schemes for a valid URL.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorUrl(const QString &field, Constraints constraints = NoConstraint, const QStringList &schemes = QStringList(), const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the validator.
     */
    ~ValidatorUrl() override;

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value converted into QUrl.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorUrl)
    Q_DISABLE_COPY(ValidatorUrl)
};

} // namespace Cutelyst

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::ValidatorUrl::Constraints)

#endif // CUTELYSTVALIDATORURL_H
