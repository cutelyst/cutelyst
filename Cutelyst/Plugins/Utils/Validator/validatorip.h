/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORIP_H
#define CUTELYSTVALIDATORIP_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorIpPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorIp validatorip.h <Cutelyst/Plugins/Utils/validatorip.h>
 * \brief Checks if the field value is a valid IP address.
 *
 * This uses QHostAddress internally to check if the \a field contains a valid IP address. You can
 * use the \a constraints flags to limit the validator to specific address ranges.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorIp : public ValidatorRule
{
public:
    /*!
     * \brief Acceptable address ranges.
     */
    enum Constraint {
        NoConstraint    = 0,  /**< No address range limit. */
        IPv4Only        = 1,  /**< Only IPv4 addresses are valid. */
        IPv6Only        = 2,  /**< Only IPv6 addresses are valid. */
        NoPrivateRange  = 4,  /**< Addresses from private networks like 192.168.0.0/12 and fe80::/10 are invalid. */
        NoReservedRange = 8,  /**< Addresses from reserved networks like 192.88.99.0/24 and 2001:db8::/32 are invalid. */
        NoMultiCast     = 16, /**< Multicast addresses are invalid. */
        PublicOnly      = 32  /**< Combines NoPrivateRange, NoReservedRange and NoMultiCast. */
    };
    Q_DECLARE_FLAGS(Constraints, Constraint)

    /*!
     * \brief Constructs a new ip validator.
     * \param field         Name of the input field to validate.
     * \param constraints   Optional validation constraints.
     * \param messages      Custom error message if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorIp(const QString &field, Constraints constraints = NoConstraint, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the ip validator.
     */
    ~ValidatorIp() override;

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a value is a valid IP address within the \a constraints.
     * \param value         The value to validate.
     * \param constraints   Optional validation constraints.
     * \return \c true if \a value is a valid IP address within the \a constraints.
     */
    static bool validate(const QString &value, Constraints constraints = NoConstraint);

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
    Q_DECLARE_PRIVATE(ValidatorIp)
    Q_DISABLE_COPY(ValidatorIp)
};

} // namespace Cutelyst

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::ValidatorIp::Constraints)

#endif // CUTELYSTVALIDATORIP_H
