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
#ifndef CUTELYSTVALIDATORIP_H
#define CUTELYSTVALIDATORIP_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"
#include <QObject>

namespace Cutelyst {
    
class ValidatorIpPrivate;

/*!
 * \brief Checks if the field value is a valid IP address.
 *
 * This uses QHostAddress internally to check if the \a field contains a valid IP address. You can
 * use the \a constraints flags to limit the validator to specific address ranges.
 *
 * If ValidatorRule::trimBefore() is set to \c true (the default), whitespaces will be removed from
 * the beginning and the end of the input value before validation. If the \a field's value is empty or if
 * the \a field is missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be present and not empty.
 *
 * \link Validator See Validator for general usage of validators. \endlink
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorIp : public ValidatorRule
{
    Q_OBJECT
public:
    /*!
     * \brief Acceptable address ranges.
     */
    enum Constraint {
        NoConstraint    = 0,    /**< No address range limit. */
        IPv4Only        = 1,    /**< Only IPv4 addresses are valid. */
        IPv6Only        = 2,    /**< Only IPv6 addresses are valid. */
        NoPrivateRange  = 4,    /**< Addresses from private networks like 192.168.0.0/12 and fe80::/10 are invalid. */
        NoReservedRange = 8,    /**< Addresses from reserved networks like 192.88.99.0/24 and 2001:db8::/32 are invalid. */
        NoMultiCast     = 16,   /**< Multicast addresses are invalid. */
        PublicOnly      = 32    /**< Combines NoPrivateRange, NoReservedRange and NoMultiCast. */
    };
    Q_DECLARE_FLAGS(Constraints, Constraint)

    /*!
     * \brief Constructs a new ip validator.
     * \param field         Name of the input field to validate.
     * \param constraint    Optional validation constraints.
     * \param label         Human readable input field label, used for error messages.
     * \param customError   Custom errror message if validation fails.
     * \param parent        Parent object.
     */
    ValidatorIp(const QString &field, Constraints constraints = NoConstraint, const QString &label = QString(), const QString &customError = QString(), QObject *parent = nullptr);
    
    /*!
     * \brief Deconstructs the ip validator.
     */
    ~ValidatorIp();
    
    /*!
     * \brief Performs the validation.
     *
     * Returns \c true on success.
     */
    bool validate() override;

    /*!
     * \brief Sets optional validation contraints.
     */
    void setConstraints(Constraints constraints);
    
protected:
    /*!
     * \brief Returns a generic error message.
     */
    QString genericErrorMessage() const override;
    
    ValidatorIp(ValidatorIpPrivate &dd, QObject *parent);
    
private:
    Q_DECLARE_PRIVATE(ValidatorIp)
    Q_DISABLE_COPY(ValidatorIp)
};
    
}

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::ValidatorIp::Constraints)

#endif //CUTELYSTVALIDATORIP_H

