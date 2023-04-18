/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATOREMAIL_H
#define CUTELYSTVALIDATOREMAIL_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorEmailPrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorEmail validatoremail.h <Cutelyst/Plugins/Utils/validatoremail.h>
 * \brief Checks if the value is a valid email address according to specific RFCs.
 *
 * You can use a \link ValidatorEmail::Category Category\endlink as threshold to define which level of compliance you accpet as valid.
 * The default threshold RFC5321 for example will only allow email addresses that can be sent without modification through SMTP. If
 * the address would contain comments like <code>(main address)test\@example.com</code>, it would not be valid because the \link ValidatorEmail::CFWSComment
 * CFWSComment\endlink diagnose is above the threshold. If it would contain a quoted string like <code>"main test address"\@example.com</code> it would be
 * valid because the \link ValidatorEmail::RFC531QuotedString RFC5321QuotedString\endlink diagnose is under the threshold.
 *
 * The parser used to validate the email address is a reimplementation of Dominic Sayers’ <a href="https://github.com/dominicsayers/isemail">isemail</a> PHP parser.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorEmail : public ValidatorRule
{
    Q_GADGET
public:
    /*!
     * \brief Validation category, used as threshold to define valid addresses.
     */
    enum Category : quint8 {
        Valid      = 1,   /**< Address is completely valid. */
        DNSWarn    = 7,   /**< Address is valid but a DNS check was not successful. Diagnose in this category is only returned if \a checkDns ist set to \c true. */
        RFC5321    = 15,  /**< Address is valid for SMTP according to <a href="https://tools.ietf.org/html/rfc5321">RFC 5321</a> but has unusual Elements. */
        CFWS       = 31,  /**< Address is valid within the message but can not be used unmodified for the envelope. */
        Deprecated = 63,  /**< Address contains deprecated elements but may still be valid in restricted contexts. */
        RFC5322    = 127, /**< Address is only valid according to the broad definition of <a href="https://tools.ietf.org/html/rfc5322">RFC 5322</a>. It is otherwise invalid. */
        Error      = 255  /**< Address is invalid for any purpose. */
    };
    Q_ENUM(Category)

    /*!
     * \brief Single diagnose values that show why an address is not valid.
     */
    enum Diagnose : quint8 {
        // Address is valid
        ValidAddress = 0, /**< Address is valid. Please note that this does not mean the address actually exists, nor even that the domain actually exists. This address could be issued by the domain owner without breaking the rules of any RFCs. */
        // Address is valid but a DNS check was not successful
        DnsWarnNoMxRecord = 5, /**< Couldn't find a MX record for this domain but an A-record does exist. */
        DnsWarnNoRecord   = 6, /**< Could neither find a MX record nor an A-record for this domain. */
        // Address is valid for SMTP but has unusual Elements
        RFC5321TLD            = 9,  /**< Address is valid but at a Top Level Domain. */
        RFC5321TLDNumberic    = 10, /**< Address is valid but the Top Level Domain begins with a number. */
        RFC5321QuotedString   = 11, /**< Address is valid but contains a quoted string. */
        RFC5321AddressLiteral = 12, /**< Address is valid but a literal address not a domain. */
        RFC5321IPv6Deprecated = 13, /**< Address is valid but contains a :: that only elides one zero group. All implementations must accept and be able to handle any legitimate <a href="https://tools.ietf.org/html/rfc4291">RFC 4291</a> format. */
        // Address is valid within the message but cannot be used unmodified for the envelope
        CFWSComment = 17, /**< Address contains comments. */
        CFWSFWS     = 18, /**< Address contains Folding White Space. */
        // Address contains deprecated elements but may still be valid in restricted contexts
        DeprecatedLocalpart  = 33, /**< The local part is in a deprecated form. */
        DeprecatedFWS        = 34, /**< Address contains an obsolete form of Folding White Space. */
        DeprecatedQText      = 35, /**< A quoted string contains a deprecated character. */
        DeprecatedQP         = 36, /**< A quoted pair contains a deprecated character. */
        DeprecatedComment    = 37, /**< Address contains a comment in a position that is deprecated. */
        DeprecatedCText      = 38, /**< A comment contains a deprecated character. */
        DeprecatedCFWSNearAt = 49, /**< Address contains a comment or Folding White Space around the @ sign. */
        // The address in only valid according to the broad definition of RFC 5322. It is otherwise invalid
        RFC5322Domain         = 65, /**< Address is <a href="https://tools.ietf.org/html/rfc5322">RFC 5322</a> compliant but contains domain characters that are not allowed by DNS. */
        RFC5322TooLong        = 66, /**< Address is too long. */
        RFC5322LocalTooLong   = 67, /**< The local part of the address is too long. */
        RFC5322DomainTooLong  = 68, /**< The domain part is too long. */
        RFC5322LabelTooLong   = 69, /**< The domain part contains an element that is too long. */
        RFC5322DomainLiteral  = 70, /**< The domain literal is not a valid <a href="https://tools.ietf.org/html/rfc5321">RFC 5321</a> address literal. */
        RFC5322DomLitOBSDText = 71, /**< The domain literal is not a valid <a href="https://tools.ietf.org/html/rfc5321">RFC 5321</a> address literal and it contains obsolete characters. */
        RFC5322IPv6GroupCount = 72, /**< The IPv6 literal address contains the wrong number of groups. */
        RFC5322IPv62x2xColon  = 73, /**< The IPv6 literal address contains too many :: sequences. */
        RFC5322IPv6BadChar    = 74, /**< The IPv6 address contains an illegal group of characters. */
        RFC5322IPv6MaxGroups  = 75, /**< The IPv6 address has too many groups. */
        RFC5322IPv6ColonStart = 76, /**< IPv6 address starts with a single colon. */
        RFC5322IPv6ColonEnd   = 77, /**< IPv6 address ends with a single colon. */
        // Address is invalid for any purpose
        ErrorExpectingDText     = 129, /**< A domain literal contains a character that is not allowed. */
        ErrorNoLocalPart        = 130, /**< Address has no local part. */
        ErrorNoDomain           = 131, /**< Address has no domain part. */
        ErrorConsecutiveDots    = 132, /**< The address may not contain consecutive dots. */
        ErrorATextAfterCFWS     = 133, /**< Address contains text after a comment or Folding White Space. */
        ErrorATextAfterQS       = 134, /**< Address contains text after a quoted string. */
        ErrorATextAfterDomLit   = 135, /**< Extra characters were found after the end of the domain literal. */
        ErrorExpectingQpair     = 136, /**< The address contains a character that is not allowed in a quoted pair. */
        ErrorExpectingAText     = 137, /**< Address contains a character that is not allowed. */
        ErrorExpectingQText     = 138, /**< A quoted string contains a character that is not allowed. */
        ErrorExpectingCText     = 139, /**< A comment contains a character that is not allowed. */
        ErrorBackslashEnd       = 140, /**< The address can't end with a backslash. */
        ErrorDotStart           = 141, /**< Neither part of the address may begin with a dot. */
        ErrorDotEnd             = 142, /**< Neither part of the address may end with a dot. */
        ErrorDomainHyphenStart  = 143, /**< A domain or subdomain cannot begin with a hyphen. */
        ErrorDomainHyphenEnd    = 144, /**< A domain or subdomain cannot end with a hyphen. */
        ErrorUnclosedQuotedStr  = 145, /**< Unclosed quoted string. */
        ErrorUnclosedComment    = 146, /**< Unclosed comment. */
        ErrorUnclosedDomLiteral = 147, /**< Domain literal is missing its closing bracket. */
        ErrorFWSCRLFx2          = 148, /**< Folding White Space contains consecutive CRLF sequences. */
        ErrorFWSCRLFEnd         = 149, /**< Folding White Space ends with a CRLF sequence. */
        ErrorCRnoLF             = 150, /**< Address contains a carriage return that is not followed by a line feed. */
        ErrorFatal              = 254  /**< Fatal internal error while validating the address. */
    };
    Q_ENUM(Diagnose)

    enum Option : quint8 {
        NoOption  = 0,                   /**< No option enabled, the default. */
        CheckDNS  = 1,                   /**< Enabled a DNS lookup to check if there are MX records for the mail domain. */
        UTF8Local = 2,                   /**< Allows UTF8 characters in the email address local part. */
        AllowIDN  = 4,                   /**< Allows internationalized domain names (IDN). */
        AllowUTF8 = UTF8Local | AllowIDN /**< Allows UTF8 characters in the email local part and internationalized domain names (IDN). */
    };
    Q_DECLARE_FLAGS(Options, Option)

    /*!
     * \brief Constructs a new email validator.
     * \param field         Name of the input field to validate.
     * \param options       Options for the validation process.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorEmail(const QString &field, Category threshold = RFC5321, Options options = NoOption, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the email validator.
     */
    ~ValidatorEmail() override;

    /*!
     * \brief Returns a descriptive and translated string for the \a diagnose.
     * \param c         The current Context, used for translation.
     * \param diagnose  The Diagnose to return the descriptive string for.
     * \param label     Optional label used in the diagnose string.
     * \return Descriptive and translated string for the \a diagnose.
     */
    static QString diagnoseString(Context *c, Diagnose diagnose, const QString &label = QString());

    /*!
     * \brief Returns a descriptive and translated string for the \a category.
     * \param c         The current Context, used for translation.
     * \param category  The Category to return the descriptive string for.
     * \param label     Optional label used in the category string.
     * \return Descriptive and translated string for the \a category.
     */
    static QString categoryString(Context *c, Category category, const QString &label = QString());

    /*!
     * \brief Returns the category the \a diagnose belongs to.
     * \param diagnose  The Diagnose to get the Category for.
     * \return The Category the \a diagnose belongs to.
     */
    static Category category(Diagnose diagnose);

    /*!
     * \brief Returns a descriptive and translated string for the Category the \a diagnose belongs to.
     * \param c         The current context, used for translation.
     * \param diagnose  The Diagnose to return the descriptive Category string for.
     * \param label     Optional label used in the category string.
     * \return Descriptive and translated string for the Category the \a diagnose belongs to.
     */
    static QString categoryString(Context *c, Diagnose diagnose, const QString &label = QString());

    /*!
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if \a email is a valid address according to the Category given in the \a threshold.
     * \param[in] email         The address to validate.
     * \param[in] threshold     The threshold category that limits the diagnose that is accepted as valid.
     * \param[in] options       Options for the validation process.
     * \param[out] diagnoses    If not a \c nullptr, this will contain a list of all issues found by the check, ordered from the highest to the lowest.
     * \return \c true if \a email is a valid address according to the Category given in the \a threshold.
     */
    static bool validate(const QString &email, Category threshold = RFC5321, Options options = NoOption, QList<Diagnose> *diagnoses = nullptr);

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the cleaned up email address without any comments as QString.
     * ValidatorReturnType::extra will contain a QList<Diagnose> list containing all issues found in the checked email, ordered from
     * the highest to the lowest.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorEmail)
    Q_DISABLE_COPY(ValidatorEmail)
};

} // namespace Cutelyst

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::ValidatorEmail::Options)

#endif // CUTELYSTVALIDATOREMAIL_H
