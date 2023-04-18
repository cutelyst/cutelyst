/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORDOMAIN_H
#define CUTELYSTVALIDATORDOMAIN_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorDomainPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorDomain validatordomain.h <Cutelyst/Plugins/Utils/validatordomain.h>
 * \brief Checks if the value of the input \a field contains FQDN according to RFC 1035.
 *
 * The \a field under validation must contain a fully qualified domain name according to <a href="https://tools.ietf.org/html/rfc1035">RFC 1035</a>.
 * If \a checkDNS is set to \c false, there will be no check if the domain is known to the domain name system, the validator will
 * only check conformance to RFC 1035. Internationalized domain names will first be converted into puny code according to IDNA.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDomain : public ValidatorRule
{
    Q_GADGET
public:
    /*!
     * \brief Possible diagnose information for the checked domain.
     */
    enum Diagnose : quint8 {
        Valid             = 0,  /**< The domain name is valid. If \a checkDNS has been set to \c false, this says nothing about the existence of the domain in the DNS. */
        MissingDNS        = 1,  /**< The domain name is valid according to RFC 1035, but there could be no DNS entry found for it. */
        InvalidChars      = 2,  /**< The domain name contains chars that are not allowed. */
        LabelTooLong      = 3,  /**< At least one of the domain name labels exceeds the maximum size of 63 chars. */
        TooLong           = 4,  /**< The whole domain name exceeds the maximum size of 253 chars. */
        InvalidLabelCount = 5,  /**< Not a valid domain name because it has either no labels or only the TLD. */
        EmptyLabel        = 6,  /**< At least one of the domain name labels is empty. */
        InvalidTLD        = 7,  /**< The TLD label contains characters that are not allowed. */
        DashStart         = 8,  /**< At least one label starts with a dash. */
        DashEnd           = 9,  /**< At least one label ends with a dash. */
        DigitStart        = 10, /**< At least one label starts with a digit. */
        DNSTimeout        = 11  /**< The DNS lookup took too long. */
    };
    Q_ENUM(Diagnose)

    /*!
     * \brief Constructs a new %ValidatorDomain with the given parameters.
     * \param field     Name of the input field to validate.
     * \param checkDNS  If \c true, a DNS lookup will be performed to check if the domain name exists in the domain name system.
     * \param messages  Custom error messages if validation fails.
     * \param defValKey \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorDomain(const QString &field, bool checkDNS = false, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * Deconstructs %ValidatorDomain
     */
    ~ValidatorDomain() override;

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a value is a valid domain name.
     * \param value             The value to validate.
     * \param checkDNS          If \c true, a DNS lookup will be performed to check if the domain name exists in the domain name system.
     * \param diagnose          Optional pointer to a variable that will be filled with the Diagnose that describes the error if validation fails.
     * \param extractedValue    Optional pointer to a variable that will contain the validated domain converted into ACE puny code.
     * \return \c true if the \a value is a valid domain name.
     */
    static bool validate(const QString &value, bool checkDNS, Diagnose *diagnose = nullptr, QString *extractedValue = nullptr);

    /*!
     * \brief Returns a human readable description of a Diagnose.
     * \param c         Current Context, used for translations.
     * \param diagnose  The Diagnose to get the description for.
     * \param label     Optinonal label that will be part of the diagnose string if not empty.
     * \return a human readable diagnose description.
     */
    static QString diagnoseString(Context *c, Diagnose diagnose, const QString &label = QString());

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as ACE version of the domain in a QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     *
     * \a errorData will contain the Diagnose returned by ValidatorDomain::validate().
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorDomain)
    Q_DISABLE_COPY(ValidatorDomain)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDOMAIN_H
