/*
 * SPDX-FileCopyrightText: (C) 2018-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYSTVALIDATORDOMAIN_H
#define CUTELYSTVALIDATORDOMAIN_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorDomainPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatordomain.h>
 * \brief Checks if the value of the input \a field contains a FQDN according to RFC 1035.
 *
 * The \a field under validation must contain a fully qualified domain name according to <a
 * href="https://tools.ietf.org/html/rfc1035">RFC 1035</a>. If \a checkDNS is set to \c false, there
 * will be no check if the domain is known to the domain name system, the validator will only check
 * conformance to RFC 1035. Internationalized domain names will first be converted into puny code
 * according to IDNA.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QString with the ACE version of the
 * domain name.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDomain : public ValidatorRule
{
    Q_GADGET
public:
    /**
     * \brief Possible diagnose information for the checked domain.
     */
    enum Diagnose : quint8 {
        Valid = 0, /**< The domain name is valid. If \a checkDNS has been set to \c false, this says
                      nothing about the existence of the domain in the DNS. */
        MissingDNS = 1,   /**< The domain name is valid according to RFC 1035, but there could be no
                             DNS entry found for it. */
        InvalidChars = 2, /**< The domain name contains chars that are not allowed. */
        LabelTooLong =
            3, /**< At least one of the domain name labels exceeds the maximum size of 63 chars. */
        TooLong = 4, /**< The whole domain name exceeds the maximum size of 253 chars. */
        InvalidLabelCount =
            5, /**< Not a valid domain name because it has either no labels or only the TLD. */
        EmptyLabel = 6,  /**< At least one of the domain name labels is empty. */
        InvalidTLD = 7,  /**< The TLD label contains characters that are not allowed. */
        DashStart  = 8,  /**< At least one label starts with a dash. */
        DashEnd    = 9,  /**< At least one label ends with a dash. */
        DigitStart = 10, /**< At least one label starts with a digit. */
        DNSTimeout = 11, /**< The DNS lookup took too long. */
        DNSError   = 12  /**< Other DNS errors than timeouts or not existing domain (NXDOMAIN). */
    };
    Q_ENUM(Diagnose)

    /**
     * \brief Options for the domain validation.
     * \since Cutelyst 5.0.0
     */
    enum Option {
        NoOption        = 0, /**< No special option selected. */
        CheckARecord    = 1, /**< Check if there is an A record for the domain. */
        CheckAAAARecord = 2, /**< Check if there is an AAAA record for the domain. */
        CheckDNS = CheckARecord | CheckAAAARecord, /**< Check if there are bot, A and AAAA records
                                                      for the domain. */
        FollowCname = 4 /**< Follow CNAME records when checking for A and/or AAAA records. */
    };
    Q_DECLARE_FLAGS(Options, Option)

    /**
     * \brief Constructs a new %ValidatorDomain object with the given parameters.
     * \since Cutelyst 5.0.0
     * \param field     Name of the input field to validate.
     * \param options   Options used for checking the domain. When checking DNS entries, it is
     *                  highly recommended to use the validator in a coroutine context.
     * \param messages  Custom error messages if validation fails.
     * \param defValKey \link Context::stash() Stash \endlink key containing a default value if
     *                  input field is empty. This value will \b NOT be validated.
     */
    explicit ValidatorDomain(const QString &field,
                             Options options                   = NoOption,
                             const ValidatorMessages &messages = {},
                             const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorDomain object.
     */
    ~ValidatorDomain() override;

    /**
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a value is a valid fully qualified domain name.
     * \note This will not perform any DNS lookup. For DNS lookups, use
     * ValidatorDomain::validateCb()
     * \since Cutelyst 5.0.0
     * \param value             The value to validate.
     * \param diagnose          Optional pointer to a variable that will be filled with the
     *                          Diagnose that describes the error if validation fails.
     * \param extractedValue    Optional pointer to a variable that will contain the validated
     *                          domain converted into ACE puny code.
     * \return \c true if the \a value is a valid domain name.
     */
    static bool validate(const QString &value,
                         Diagnose *diagnose      = nullptr,
                         QString *extractedValue = nullptr);

    /**
     * \ingroup plugins-utils-validator-rules
     * \brief Checks if \a value is a vaid fully qualified domain name and writes the result
     * to the callback \a cb.
     * \since Cutelyst 5.0.0
     * \param value      The value to validate.
     * \param options    Options to use for the validation.
     * \param cb         Callback funcion that will be called for the result.
     */
    static void
        validateCb(const QString &value,
                   Options options,
                   std::function<void(Diagnose diagnose, const QString &extractedValue)> cb);

    /**
     * Returns a human readable description of a Diagnose.
     *
     * \param c         Current Context, used for translations.
     * \param diagnose  The Diagnose to get the description for.
     * \param label     Optinonal label that will be part of the diagnose string if not empty.
     * \return a human readable diagnose description.
     */
    static QString diagnoseString(Context *c, Diagnose diagnose, const QString &label = {});

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as ACE version of the domain in a QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter
     * value as ACE version of the domain in a QString.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * \brief Returns a generic error message if validation failed.
     *
     * \a errorData will contain the Diagnose returned by ValidatorDomain::validate().
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorDomain) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorDomain)
};

} // namespace Cutelyst

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::ValidatorDomain::Options);

#endif // CUTELYSTVALIDATORDOMAIN_H
