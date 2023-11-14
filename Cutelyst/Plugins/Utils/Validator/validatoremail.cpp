/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoremail_p.h"

#include <algorithm>
#include <functional>

#include <QDnsLookup>
#include <QEventLoop>
#include <QTimer>
#include <QUrl>

using namespace Cutelyst;

const QRegularExpression ValidatorEmailPrivate::ipv4Regex{
    u"\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25["
    "0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$"_qs};
const QRegularExpression ValidatorEmailPrivate::ipv6PartRegex{u"^[0-9A-Fa-f]{0,4}$"_qs};
const QString ValidatorEmailPrivate::stringSpecials{u"()<>[]:;@\\,.\""_qs};

ValidatorEmail::ValidatorEmail(const QString &field,
                               Category threshold,
                               Options options,
                               const Cutelyst::ValidatorMessages &messages,
                               const QString &defValKey)
    : ValidatorRule(*new ValidatorEmailPrivate(field, threshold, options, messages, defValKey))
{
}

ValidatorEmail::~ValidatorEmail() = default;

ValidatorReturnType ValidatorEmail::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    Q_D(const ValidatorEmail);

    if (!v.isEmpty()) {

        //        QString email;
        //        const int atPos = v.lastIndexOf(QLatin1Char('@'));
        //        if (atPos > 0) {
        //            const QStringRef local = v.leftRef(atPos);
        //            const QString domain = v.mid(atPos + 1);
        //            bool asciiDomain = true;
        //            for (const QChar &ch : domain) {
        //                const ushort &uc = ch.unicode();
        //                if (uc > 127) {
        //                    asciiDomain = false;
        //                    break;
        //                }
        //            }

        //            if (asciiDomain) {
        //                email = v;
        //            } else {
        //                email = local + QLatin1Char('@') +
        //                QString::fromLatin1(QUrl::toAce(domain));
        //            }
        //        } else {
        //            email = v;
        //        }

        ValidatorEmailDiagnoseStruct diag;

        if (ValidatorEmailPrivate::checkEmail(v, d->options, d->threshold, &diag)) {
            if (!diag.literal.isEmpty()) {
                result.value.setValue<QString>(diag.localpart + QLatin1Char('@') + diag.literal);
            } else {
                result.value.setValue<QString>(diag.localpart + QLatin1Char('@') + diag.domain);
            }
        } else {
            result.errorMessage =
                validationError(c, QVariant::fromValue<Diagnose>(diag.finalStatus));
        }

        result.extra = QVariant::fromValue<QList<Diagnose>>(diag.returnStatus);

    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorEmail::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    error = ValidatorEmail::diagnoseString(c, errorData.value<Diagnose>(), label(c));

    return error;
}

bool ValidatorEmailPrivate::checkEmail(const QString &address,
                                       ValidatorEmail::Options options,
                                       ValidatorEmail::Category threshold,
                                       ValidatorEmailDiagnoseStruct *diagnoseStruct)
{
    QList<ValidatorEmail::Diagnose> returnStatus{ValidatorEmail::ValidAddress};

    EmailPart context = ComponentLocalpart;
    QList<EmailPart> contextStack{context};
    EmailPart contextPrior = ComponentLocalpart;

    QChar token;
    QChar tokenPrior;

    QString parseLocalPart;
    QString parseDomain;
    QString parseLiteral;
    QMap<int, QString> atomListLocalPart;
    QMap<int, QString> atomListDomain;
    int elementCount = 0;
    int elementLen   = 0;
    bool hypenFlag   = false;
    bool endOrDie    = false;
    int crlf_count   = 0;

    const bool checkDns       = options.testFlag(ValidatorEmail::CheckDNS);
    const bool allowUtf8Local = options.testFlag(ValidatorEmail::UTF8Local);
    const bool allowIdn       = options.testFlag(ValidatorEmail::AllowIDN);

    QString email;
    const qsizetype atPos = address.lastIndexOf(QLatin1Char('@'));
    if (allowIdn) {
        if (atPos > 0) {
            const QString local  = address.left(atPos);
            const QString domain = address.mid(atPos + 1);
            bool asciiDomain     = true;
            for (const QChar &ch : domain) {
                const ushort &uc = ch.unicode();
                if (uc > ValidatorEmailPrivate::asciiEnd) {
                    asciiDomain = false;
                    break;
                }
            }

            if (asciiDomain) {
                email = address;
            } else {
                email = local + QLatin1Char('@') + QString::fromLatin1(QUrl::toAce(domain));
            }
        } else {
            email = address;
        }
    } else {
        email = address;
    }

    const qsizetype rawLength = email.length();

    for (int i = 0; i < rawLength; i++) {
        token = email[i];

        switch (context) {
        //-------------------------------------------------------------
        // local-part
        //-------------------------------------------------------------
        case ComponentLocalpart:
        {
            // https://tools.ietf.org/html/rfc5322#section-3.4.1
            //   local-part      =   dot-atom / quoted-string / obs-local-part
            //
            //   dot-atom        =   [CFWS] dot-atom-text [CFWS]
            //
            //   dot-atom-text   =   1*atext *("." 1*atext)
            //
            //   quoted-string   =   [CFWS]
            //                       DQUOTE *([FWS] qcontent) [FWS] DQUOTE
            //                       [CFWS]
            //
            //   obs-local-part  =   word *("." word)
            //
            //   word            =   atom / quoted-string
            //
            //   atom            =   [CFWS] 1*atext [CFWS]

            if (token == QLatin1Char('(')) { // comment
                if (elementLen == 0) {
                    // Comments are OK at the beginning of an element
                    returnStatus.push_back((elementCount == 0) ? ValidatorEmail::CFWSComment
                                                               : ValidatorEmail::DeprecatedComment);
                } else {
                    returnStatus.push_back(ValidatorEmail::CFWSComment);
                    endOrDie = true; // We can't start a comment in the middle of an element, so
                                     // this better be the end
                }

                contextStack.push_back(context);
                context = ContextComment;
            } else if (token == QLatin1Char('.')) { // Next dot-atom element
                if (elementLen == 0) {
                    // Another dot, already?
                    returnStatus.push_back((elementCount == 0)
                                               ? ValidatorEmail::ErrorDotStart
                                               : ValidatorEmail::ErrorConsecutiveDots);
                } else {
                    // The entire local part can be a quoted string for RFC 5321
                    // If it's just one atom that is quoten then it's an RFC 5322 obsolete form
                    if (endOrDie) {
                        returnStatus.push_back(ValidatorEmail::DeprecatedLocalpart);
                    }
                }

                endOrDie = false; // CFWS & quoted strings are OK again now we're at the beginning
                                  // of an element (although they are obsolete forms)
                elementLen = 0;
                elementCount++;
                parseLocalPart += token;
                atomListLocalPart[elementCount] = QString();
            } else if (token == QLatin1Char('"')) {
                if (elementLen == 0) {
                    // The entire local-part can be a quoted string for RFC 5321
                    // If it's just one atom that is quoted then it's an RFC 5322 obsolete form
                    returnStatus.push_back((elementCount == 0)
                                               ? ValidatorEmail::RFC5321QuotedString
                                               : ValidatorEmail::DeprecatedLocalpart);

                    parseLocalPart += token;
                    atomListLocalPart[elementCount] += token;
                    elementLen++;
                    endOrDie = true; // quoted string must be the entire element
                    contextStack.push_back(context);
                    context = ContextQuotedString;
                } else {
                    returnStatus.push_back(ValidatorEmail::ErrorExpectingAText); // Fatal error
                }
            } else if ((token == QChar(QChar::CarriageReturn)) || (token == QChar(QChar::Space)) ||
                       (token == QChar(QChar::Tabulation))) { // Folding White Space
                if ((token == QChar(QChar::CarriageReturn)) &&
                    ((++i == rawLength) || (email[i] != QChar(QChar::LineFeed)))) {
                    returnStatus.push_back(ValidatorEmail::ErrorCRnoLF);
                    break;
                }

                if (elementLen == 0) {
                    returnStatus.push_back((elementCount == 0) ? ValidatorEmail::CFWSFWS
                                                               : ValidatorEmail::DeprecatedFWS);
                } else {
                    endOrDie = true; // We can't start FWS in the middle of an element, so this
                                     // better be the end
                }

                contextStack.push_back(context);
                context    = ContextFWS;
                tokenPrior = token;
            } else if (token == QLatin1Char('@')) {
                // At this point we should have a valid local part
                if (contextStack.size() != 1) {
                    returnStatus.push_back(ValidatorEmail::ErrorFatal);
                    qCCritical(C_VALIDATOR) << "ValidatorEmail: Unexpected item on context stack";
                    break;
                }

                if (parseLocalPart.isEmpty()) {
                    returnStatus.push_back(ValidatorEmail::ErrorNoLocalPart); // Fatal error
                } else if (elementLen == 0) {
                    returnStatus.push_back(ValidatorEmail::ErrorDotEnd); // Fatal Error
                } else if (parseLocalPart.size() > ValidatorEmailPrivate::maxLocalPartLength) {
                    // https://tools.ietf.org/html/rfc5321#section-4.5.3.1.1
                    // The maximum total length of a user name or other local-part is 64
                    // octets.
                    returnStatus.push_back(ValidatorEmail::RFC5322LocalTooLong);
                } else if ((contextPrior == ContextComment) || (contextPrior == ContextFWS)) {
                    // https://tools.ietf.org/html/rfc5322#section-3.4.1
                    //   Comments and folding white space
                    //   SHOULD NOT be used around the "@" in the addr-spec.
                    //
                    // https://tools.ietf.org/html/rfc2119
                    // 4. SHOULD NOT   This phrase, or the phrase "NOT RECOMMENDED" mean that
                    //    there may exist valid reasons in particular circumstances when the
                    //    particular behavior is acceptable or even useful, but the full
                    //    implications should be understood and the case carefully weighed
                    //    before implementing any behavior described with this label.
                    returnStatus.push_back(ValidatorEmail::DeprecatedCFWSNearAt);
                }

                context = ComponentDomain;
                contextStack.clear();
                contextStack.push_back(context);
                elementCount = 0;
                elementLen   = 0;
                endOrDie     = false;

            } else { // atext
                // https://tools.ietf.org/html/rfc5322#section-3.2.3
                //    atext           =   ALPHA / DIGIT /    ; Printable US-ASCII
                //                        "!" / "#" /        ;  characters not including
                //                        "$" / "%" /        ;  specials.  Used for atoms.
                //                        "&" / "'" /
                //                        "*" / "+" /
                //                        "-" / "/" /
                //                        "=" / "?" /
                //                        "^" / "_" /
                //                        "`" / "{" /
                //                        "|" / "}" /
                //
                if (endOrDie) {
                    switch (contextPrior) {
                    case ContextComment:
                    case ContextFWS:
                        returnStatus.push_back(ValidatorEmail::ErrorATextAfterCFWS);
                        break;
                    case ContextQuotedString:
                        returnStatus.push_back(ValidatorEmail::ErrorATextAfterQS);
                        break;
                    default:
                        returnStatus.push_back(ValidatorEmail::ErrorFatal);
                        qCCritical(C_VALIDATOR)
                            << "ValidatorEmail: More atext found where none is allowed, "
                               "but unrecognizes prior context";
                        break;
                    }
                } else {
                    contextPrior       = context;
                    const char16_t uni = token.unicode();

                    if (!allowUtf8Local) {
                        if ((uni < ValidatorEmailPrivate::asciiExclamationMark) ||
                            (uni > ValidatorEmailPrivate::asciiTilde) ||
                            ValidatorEmailPrivate::stringSpecials.contains(token)) {
                            returnStatus.push_back(
                                ValidatorEmail::ErrorExpectingAText); // fatal error
                        }
                    } else {
                        if (!token.isLetterOrNumber()) {
                            if ((uni < ValidatorEmailPrivate::asciiExclamationMark) ||
                                (uni > ValidatorEmailPrivate::asciiTilde) ||
                                ValidatorEmailPrivate::stringSpecials.contains(token)) {
                                returnStatus.push_back(
                                    ValidatorEmail::ErrorExpectingAText); // fatal error
                            }
                        }
                    }

                    parseLocalPart += token;
                    atomListLocalPart[elementCount] += token;
                    elementLen++;
                }
            }
        } break;
        //-----------------------------------------
        // Domain
        //-----------------------------------------
        case ComponentDomain:
        {
            // https://tools.ietf.org/html/rfc5322#section-3.4.1
            //   domain          =   dot-atom / domain-literal / obs-domain
            //
            //   dot-atom        =   [CFWS] dot-atom-text [CFWS]
            //
            //   dot-atom-text   =   1*atext *("." 1*atext)
            //
            //   domain-literal  =   [CFWS] "[" *([FWS] dtext) [FWS] "]" [CFWS]
            //
            //   dtext           =   %d33-90 /          ; Printable US-ASCII
            //                       %d94-126 /         ;  characters not including
            //                       obs-dtext          ;  "[", "]", or "\"
            //
            //   obs-domain      =   atom *("." atom)
            //
            //   atom            =   [CFWS] 1*atext [CFWS]
            // https://tools.ietf.org/html/rfc5321#section-4.1.2
            //   Mailbox        = Local-part "@" ( Domain / address-literal )
            //
            //   Domain         = sub-domain *("." sub-domain)
            //
            //   address-literal  = "[" ( IPv4-address-literal /
            //                    IPv6-address-literal /
            //                    General-address-literal ) "]"
            //                    ; See Section 4.1.3
            // https://tools.ietf.org/html/rfc5322#section-3.4.1
            //      Note: A liberal syntax for the domain portion of addr-spec is
            //      given here.  However, the domain portion contains addressing
            //      information specified by and used in other protocols (e.g.,
            //      [RFC1034], [RFC1035], [RFC1123], [RFC5321]).  It is therefore
            //      incumbent upon implementations to conform to the syntax of
            //      addresses for the context in which they are used.
            // is_email() author's note: it's not clear how to interpret this in
            // the context of a general email address validator. The conclusion I
            // have reached is this: "addressing information" must comply with
            // RFC 5321 (and in turn RFC 1035), anything that is "semantically
            // invisible" must comply only with RFC 5322.

            if (token == QLatin1Char('(')) { // comment
                if (elementLen == 0) {
                    // Comments at the start of the domain are deprecated in the text
                    // Comments at the start of a subdomain are obs-domain
                    // (https://tools.ietf.org/html/rfc5322#section-3.4.1)
                    returnStatus.push_back((elementCount == 0)
                                               ? ValidatorEmail::DeprecatedCFWSNearAt
                                               : ValidatorEmail::DeprecatedComment);
                } else {
                    returnStatus.push_back(ValidatorEmail::CFWSComment);
                    endOrDie = true; // We can't start a comment in the middle of an element, so
                                     // this better be the end
                }

                contextStack.push_back(context);
                context = ContextComment;
            } else if (token == QLatin1Char('.')) { // next dot-atom element
                if (elementLen == 0) {
                    // another dot, already?
                    returnStatus.push_back((elementCount == 0)
                                               ? ValidatorEmail::ErrorDotStart
                                               : ValidatorEmail::ErrorConsecutiveDots);
                } else if (hypenFlag) {
                    // Previous subdomain ended in a hyphen
                    returnStatus.push_back(ValidatorEmail::ErrorDomainHyphenEnd); // fatal error
                } else {
                    // Nowhere in RFC 5321 does it say explicitly that the
                    // domain part of a Mailbox must be a valid domain according
                    // to the DNS standards set out in RFC 1035, but this *is*
                    // implied in several places. For instance, wherever the idea
                    // of host routing is discussed the RFC says that the domain
                    // must be looked up in the DNS. This would be nonsense unless
                    // the domain was designed to be a valid DNS domain. Hence we
                    // must conclude that the RFC 1035 restriction on label length
                    // also applies to RFC 5321 domains.
                    //
                    // https://tools.ietf.org/html/rfc1035#section-2.3.4
                    // labels          63 octets or less
                    if (elementLen > ValidatorEmailPrivate::maxDnsLabelLength) {
                        returnStatus.push_back(ValidatorEmail::RFC5322LabelTooLong);
                    }
                }

                endOrDie = false; // CFWS is OK again now we're at the beginning of an element
                                  // (although it may be obsolete CFWS)
                elementLen = 0;
                elementCount++;
                atomListDomain[elementCount] = QString();
                parseDomain += token;

            } else if (token == QLatin1Char('[')) { // Domain literal
                if (parseDomain.isEmpty()) {
                    endOrDie = true; // domain literal must be the only component
                    elementLen++;
                    contextStack.push_back(context);
                    context = ComponentLiteral;
                    parseDomain += token;
                    atomListDomain[elementCount] += token;
                    parseLiteral = QString();
                } else {
                    returnStatus.push_back(ValidatorEmail::ErrorExpectingAText); // Fatal error
                }
            } else if ((token == QChar(QChar::CarriageReturn)) || (token == QChar(QChar::Space)) ||
                       (token == QChar(QChar::Tabulation))) { // Folding White Space
                if ((token == QChar(QChar::CarriageReturn)) &&
                    ((++i == rawLength) || email[i] != QChar(QChar::LineFeed))) {
                    returnStatus.push_back(ValidatorEmail::ErrorCRnoLF); // Fatal error
                    break;
                }

                if (elementLen == 0) {
                    returnStatus.push_back((elementCount == 0)
                                               ? ValidatorEmail::DeprecatedCFWSNearAt
                                               : ValidatorEmail::DeprecatedFWS);
                } else {
                    returnStatus.push_back(ValidatorEmail::CFWSFWS);
                    endOrDie = true; // We can't start FWS in the middle of an element, so this
                                     // better be the end
                }

                contextStack.push_back(context);
                context    = ContextFWS;
                tokenPrior = token;

            } else { // atext
                // RFC 5322 allows any atext...
                // https://tools.ietf.org/html/rfc5322#section-3.2.3
                //    atext           =   ALPHA / DIGIT /    ; Printable US-ASCII
                //                        "!" / "#" /        ;  characters not including
                //                        "$" / "%" /        ;  specials.  Used for atoms.
                //                        "&" / "'" /
                //                        "*" / "+" /
                //                        "-" / "/" /
                //                        "=" / "?" /
                //                        "^" / "_" /
                //                        "`" / "{" /
                //                        "|" / "}" /
                //                        "~"
                // But RFC 5321 only allows letter-digit-hyphen to comply with DNS rules (RFCs 1034
                // & 1123) https://tools.ietf.org/html/rfc5321#section-4.1.2
                //   sub-domain     = Let-dig [Ldh-str]
                //
                //   Let-dig        = ALPHA / DIGIT
                //
                //   Ldh-str        = *( ALPHA / DIGIT / "-" ) Let-dig
                //

                if (endOrDie) {
                    // We have encountered atext where it is no longer valid
                    switch (contextPrior) {
                    case ContextComment:
                    case ContextFWS:
                        returnStatus.push_back(ValidatorEmail::ErrorATextAfterCFWS);
                        break;
                    case ComponentLiteral:
                        returnStatus.push_back(ValidatorEmail::ErrorATextAfterDomLit);
                        break;
                    default:
                        returnStatus.push_back(ValidatorEmail::ErrorFatal);
                        qCCritical(C_VALIDATOR)
                            << "ValidatorEmail: More atext found where none is allowed, but"
                            << "unrecognised prior context.";
                        break;
                    }
                }

                const char16_t uni = token.unicode();
                hypenFlag = false; // Assume this token isn't a hyphen unless we discover it is

                if ((uni < ValidatorEmailPrivate::asciiExclamationMark) ||
                    (uni > ValidatorEmailPrivate::asciiTilde) ||
                    ValidatorEmailPrivate::stringSpecials.contains(token)) {
                    returnStatus.push_back(ValidatorEmail::ErrorExpectingAText); // Fatal error
                } else if (token == QLatin1Char('-')) {
                    if (elementLen == 0) {
                        // Hyphens can't be at the beggining of a subdomain
                        returnStatus.push_back(
                            ValidatorEmail::ErrorDomainHyphenStart); // Fatal error
                    }
                    hypenFlag = true;
                } else if (!(((uni >= ValidatorRulePrivate::ascii_0) &&
                              (uni <= ValidatorRulePrivate::ascii_9)) ||
                             ((uni >= ValidatorRulePrivate::ascii_A) &&
                              (uni <= ValidatorRulePrivate::ascii_Z)) ||
                             ((uni >= ValidatorRulePrivate::ascii_a) &&
                              (uni <= ValidatorRulePrivate::ascii_z)))) {
                    // NOt an RFC 5321 subdomain, but still ok by RFC 5322
                    returnStatus.push_back(ValidatorEmail::RFC5322Domain);
                }

                parseDomain += token;
                atomListDomain[elementCount] += token;
                elementLen++;
            }
        } break;
        //-------------------------------------------------------------
        // Domain literal
        //-------------------------------------------------------------
        case ComponentLiteral:
        {
            // https://tools.ietf.org/html/rfc5322#section-3.4.1
            //   domain-literal  =   [CFWS] "[" *([FWS] dtext) [FWS] "]" [CFWS]
            //
            //   dtext           =   %d33-90 /          ; Printable US-ASCII
            //                       %d94-126 /         ;  characters not including
            //                       obs-dtext          ;  "[", "]", or "\"
            //
            //   obs-dtext       =   obs-NO-WS-CTL / quoted-pair
            if (token == QLatin1Char(']')) { // End of domain literal
                if (static_cast<int>(
                        *std::max_element(returnStatus.constBegin(), returnStatus.constEnd())) <
                    static_cast<int>(ValidatorEmail::Deprecated)) {
                    // Could be a valid RFC 5321 address literal, so let's check

                    // https://tools.ietf.org/html/rfc5321#section-4.1.2
                    //   address-literal  = "[" ( IPv4-address-literal /
                    //                    IPv6-address-literal /
                    //                    General-address-literal ) "]"
                    //                    ; See Section 4.1.3
                    //
                    // https://tools.ietf.org/html/rfc5321#section-4.1.3
                    //   IPv4-address-literal  = Snum 3("."  Snum)
                    //
                    //   IPv6-address-literal  = "IPv6:" IPv6-addr
                    //
                    //   General-address-literal  = Standardized-tag ":" 1*dcontent
                    //
                    //   Standardized-tag  = Ldh-str
                    //                     ; Standardized-tag MUST be specified in a
                    //                     ; Standards-Track RFC and registered with IANA
                    //
                    //   dcontent       = %d33-90 / ; Printable US-ASCII
                    //                  %d94-126 ; excl. "[", "\", "]"
                    //
                    //   Snum           = 1*3DIGIT
                    //                  ; representing a decimal integer
                    //                  ; value in the range 0 through 255
                    //
                    //   IPv6-addr      = IPv6-full / IPv6-comp / IPv6v4-full / IPv6v4-comp
                    //
                    //   IPv6-hex       = 1*4HEXDIG
                    //
                    //   IPv6-full      = IPv6-hex 7(":" IPv6-hex)
                    //
                    //   IPv6-comp      = [IPv6-hex *5(":" IPv6-hex)] "::"
                    //                  [IPv6-hex *5(":" IPv6-hex)]
                    //                  ; The "::" represents at least 2 16-bit groups of
                    //                  ; zeros.  No more than 6 groups in addition to the
                    //                  ; "::" may be present.
                    //
                    //   IPv6v4-full    = IPv6-hex 5(":" IPv6-hex) ":" IPv4-address-literal
                    //
                    //   IPv6v4-comp    = [IPv6-hex *3(":" IPv6-hex)] "::"
                    //                  [IPv6-hex *3(":" IPv6-hex) ":"]
                    //                  IPv4-address-literal
                    //                  ; The "::" represents at least 2 16-bit groups of
                    //                  ; zeros.  No more than 4 groups in addition to the
                    //                  ; "::" and IPv4-address-literal may be present.
                    //
                    // is_email() author's note: We can't use ip2long() to validate
                    // IPv4 addresses because it accepts abbreviated addresses
                    // (xxx.xxx.xxx), expanding the last group to complete the address.
                    // filter_var() validates IPv6 address inconsistently (up to PHP 5.3.3
                    // at least) -- see https://bugs.php.net/bug.php?id=53236 for example

                    int maxGroups          = 8; // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                    qsizetype index        = -1;
                    QString addressLiteral = parseLiteral;

                    const QRegularExpressionMatch ipv4Match =
                        ValidatorEmailPrivate::ipv4Regex.match(addressLiteral);
                    if (ipv4Match.hasMatch()) {
                        index = addressLiteral.lastIndexOf(ipv4Match.captured());
                        if (index != 0) {
                            addressLiteral =
                                addressLiteral.mid(0, index) +
                                QLatin1String(
                                    "0:0"); // Convert IPv4 part to IPv6 format for further testing
                        }
                    }

                    if (index == 0) {
                        // Nothing there except a valid IPv4 address, so...
                        returnStatus.push_back(ValidatorEmail::RFC5321AddressLiteral);
                    } else if (QString::compare(
                                   addressLiteral.left(5),
                                   QLatin1String(
                                       "IPv6:")) != // NOLINT(cppcoreguidelines-avoid-magic-numbers)
                               0) {
                        returnStatus.push_back(ValidatorEmail::RFC5322DomainLiteral);
                    } else {
                        const QString ipv6          = addressLiteral.mid(5);
                        const QStringList matchesIP = ipv6.split(QLatin1Char(':'));
                        qsizetype groupCount        = matchesIP.size();
                        index                       = ipv6.indexOf(QLatin1String("::"));

                        if (index < 0) {
                            // We need exactly the right number of groups
                            if (groupCount != maxGroups) {
                                returnStatus.push_back(ValidatorEmail::RFC5322IPv6GroupCount);
                            }
                        } else {
                            if (index != ipv6.lastIndexOf(QLatin1String("::"))) {
                                returnStatus.push_back(ValidatorEmail::RFC5322IPv62x2xColon);
                            } else {
                                if ((index == 0) || (index == (ipv6.length() - 2))) {
                                    maxGroups++;
                                }

                                if (groupCount > maxGroups) {
                                    returnStatus.push_back(ValidatorEmail::RFC5322IPv6MaxGroups);
                                } else if (groupCount == maxGroups) {
                                    returnStatus.push_back(
                                        ValidatorEmail::RFC5321IPv6Deprecated); // Eliding a single
                                                                                // "::"
                                }
                            }
                        }

                        if ((ipv6.size() == 1 && ipv6[0] == QLatin1Char(':')) ||
                            (ipv6[0] == QLatin1Char(':') && ipv6[1] != QLatin1Char(':'))) {
                            returnStatus.push_back(
                                ValidatorEmail::RFC5322IPv6ColonStart); // Address starts with a
                                                                        // single colon
                        } else if (ipv6.right(2).at(1) == QLatin1Char(':') &&
                                   ipv6.right(2).at(0) != QLatin1Char(':')) {
                            returnStatus.push_back(
                                ValidatorEmail::RFC5322IPv6ColonEnd); // Address ends with a single
                                                                      // colon
                        } else {
                            int unmatchedChars = 0;
                            for (const QString &ip : matchesIP) {
                                if (!ip.contains(ValidatorEmailPrivate::ipv6PartRegex)) {
                                    unmatchedChars++;
                                }
                            }
                            if (unmatchedChars != 0) {
                                returnStatus.push_back(ValidatorEmail::RFC5322IPv6BadChar);
                            } else {
                                returnStatus.push_back(ValidatorEmail::RFC5321AddressLiteral);
                            }
                        }
                    }

                } else {
                    returnStatus.push_back(ValidatorEmail::RFC5322DomainLiteral);
                }

                parseDomain += token;
                atomListDomain[elementCount] += token;
                elementLen++;
                contextPrior = context;
                context      = contextStack.takeLast();
            } else if (token == QLatin1Char('\\')) {
                returnStatus.push_back(ValidatorEmail::RFC5322DomLitOBSDText);
                contextStack.push_back(context);
                context = ContextQuotedPair;
            } else if ((token == QChar(QChar::CarriageReturn)) || (token == QChar(QChar::Space)) ||
                       (token == QChar(QChar::Tabulation))) { // Folding White Space
                if ((token == QChar(QChar::CarriageReturn)) &&
                    ((++i == rawLength) || (email[i] != QChar(QChar::LineFeed)))) {
                    returnStatus.push_back(ValidatorEmail::ErrorCRnoLF); // Fatal error
                    break;
                }

                returnStatus.push_back(ValidatorEmail::CFWSFWS);
                contextStack.push_back(context);
                context    = ContextFWS;
                tokenPrior = token;

            } else { // dtext
                // https://tools.ietf.org/html/rfc5322#section-3.4.1
                //   dtext           =   %d33-90 /          ; Printable US-ASCII
                //                       %d94-126 /         ;  characters not including
                //                       obs-dtext          ;  "[", "]", or "\"
                //
                //   obs-dtext       =   obs-NO-WS-CTL / quoted-pair
                //
                //   obs-NO-WS-CTL   =   %d1-8 /            ; US-ASCII control
                //                       %d11 /             ;  characters that do not
                //                       %d12 /             ;  include the carriage
                //                       %d14-31 /          ;  return, line feed, and
                //                       %d127              ;  white space characters
                const char16_t uni = token.unicode();

                // CR, LF, SP & HTAB have already been parsed above
                if ((uni > ValidatorEmailPrivate::asciiEnd) || (uni == 0) ||
                    (uni == QLatin1Char('[').unicode())) {
                    returnStatus.push_back(ValidatorEmail::ErrorExpectingDText); // Fatal error
                    break;
                } else if ((uni < ValidatorEmailPrivate::asciiExclamationMark) ||
                           (uni == ValidatorEmailPrivate::asciiEnd)) {
                    returnStatus.push_back(ValidatorEmail::RFC5322DomLitOBSDText);
                }

                parseLiteral += token;
                parseDomain += token;
                atomListDomain[elementCount] += token;
                elementLen++;
            }
        } break;
        //-------------------------------------------------------------
        // Quoted string
        //-------------------------------------------------------------
        case ContextQuotedString:
        {
            // https://tools.ietf.org/html/rfc5322#section-3.2.4
            //   quoted-string   =   [CFWS]
            //                       DQUOTE *([FWS] qcontent) [FWS] DQUOTE
            //                       [CFWS]
            //
            //   qcontent        =   qtext / quoted-pair
            if (token == QLatin1Char('\\')) { // Quoted pair
                contextStack.push_back(context);
                context = ContextQuotedPair;
            } else if ((token == QChar(QChar::CarriageReturn)) ||
                       (token == QChar(QChar::Tabulation))) { // Folding White Space
                // Inside a quoted string, spaces are allowed as regular characters.
                // It's only FWS if we include HTAB or CRLF
                if ((token == QChar(QChar::CarriageReturn)) &&
                    ((++i == rawLength) || (email[i] != QChar(QChar::LineFeed)))) {
                    returnStatus.push_back(ValidatorEmail::ErrorCRnoLF);
                    break;
                }

                // https://tools.ietf.org/html/rfc5322#section-3.2.2
                //   Runs of FWS, comment, or CFWS that occur between lexical tokens in a
                //   structured header field are semantically interpreted as a single
                //   space character.

                // https://tools.ietf.org/html/rfc5322#section-3.2.4
                //   the CRLF in any FWS/CFWS that appears within the quoted-string [is]
                //   semantically "invisible" and therefore not part of the quoted-string

                parseLocalPart += QChar(QChar::Space);
                atomListLocalPart[elementCount] += QChar(QChar::Space);
                elementLen++;

                returnStatus.push_back(ValidatorEmail::CFWSFWS);
                contextStack.push_back(context);
                context    = ContextFWS;
                tokenPrior = token;
            } else if (token == QLatin1Char('"')) { // end of quoted string
                parseLocalPart += token;
                atomListLocalPart[elementCount] += token;
                elementLen++;
                contextPrior = context;
                context      = contextStack.takeLast();
            } else { // qtext
                // https://tools.ietf.org/html/rfc5322#section-3.2.4
                //   qtext           =   %d33 /             ; Printable US-ASCII
                //                       %d35-91 /          ;  characters not including
                //                       %d93-126 /         ;  "\" or the quote character
                //                       obs-qtext
                //
                //   obs-qtext       =   obs-NO-WS-CTL
                //
                //   obs-NO-WS-CTL   =   %d1-8 /            ; US-ASCII control
                //                       %d11 /             ;  characters that do not
                //                       %d12 /             ;  include the carriage
                //                       %d14-31 /          ;  return, line feed, and
                //                       %d127              ;  white space characters
                const char16_t uni = token.unicode();

                if (!allowUtf8Local) {
                    if ((uni > ValidatorEmailPrivate::asciiEnd) || (uni == 0) ||
                        (uni == ValidatorEmailPrivate::asciiLF)) {
                        returnStatus.push_back(ValidatorEmail::ErrorExpectingQText); // Fatal error
                    } else if ((uni < ValidatorRulePrivate::asciiSpace) ||
                               (uni == ValidatorEmailPrivate::asciiEnd)) {
                        returnStatus.push_back(ValidatorEmail::DeprecatedQText);
                    }
                } else {
                    if (!token.isLetterOrNumber()) {
                        if ((uni > ValidatorEmailPrivate::asciiEnd) || (uni == 0) ||
                            (uni == ValidatorEmailPrivate::asciiLF)) {
                            returnStatus.push_back(
                                ValidatorEmail::ErrorExpectingQText); // Fatal error
                        } else if ((uni < ValidatorRulePrivate::asciiSpace) ||
                                   (uni == ValidatorEmailPrivate::asciiEnd)) {
                            returnStatus.push_back(ValidatorEmail::DeprecatedQText);
                        }
                    }
                }

                parseLocalPart += token;
                atomListLocalPart[elementCount] += token;
                elementLen++;
            }

            // https://tools.ietf.org/html/rfc5322#section-3.4.1
            //   If the
            //   string can be represented as a dot-atom (that is, it contains no
            //   characters other than atext characters or "." surrounded by atext
            //   characters), then the dot-atom form SHOULD be used and the quoted-
            //   string form SHOULD NOT be used.
            // To do
        } break;
        //-------------------------------------------------------------
        // Quoted pair
        //-------------------------------------------------------------
        case ContextQuotedPair:
        {
            // https://tools.ietf.org/html/rfc5322#section-3.2.1
            //   quoted-pair     =   ("\" (VCHAR / WSP)) / obs-qp
            //
            //   VCHAR           =  %d33-126            ; visible (printing) characters
            //   WSP             =  SP / HTAB           ; white space
            //
            //   obs-qp          =   "\" (%d0 / obs-NO-WS-CTL / LF / CR)
            //
            //   obs-NO-WS-CTL   =   %d1-8 /            ; US-ASCII control
            //                       %d11 /             ;  characters that do not
            //                       %d12 /             ;  include the carriage
            //                       %d14-31 /          ;  return, line feed, and
            //                       %d127              ;  white space characters
            //
            // i.e. obs-qp       =  "\" (%d0-8, %d10-31 / %d127)

            const char16_t uni = token.unicode();

            if (uni > ValidatorEmailPrivate::asciiEnd) {
                returnStatus.push_back(ValidatorEmail::ErrorExpectingQpair); // Fatal error
            } else if (((uni < ValidatorEmailPrivate::asciiUS) &&
                        (uni != ValidatorRulePrivate::asciiTab)) ||
                       (uni == ValidatorEmailPrivate::asciiEnd)) {
                returnStatus.push_back(ValidatorEmail::DeprecatedQP);
            }

            // At this point we know where this qpair occurred so
            // we could check to see if the character actually
            // needed to be quoted at all.
            // https://tools.ietf.org/html/rfc5321#section-4.1.2
            //   the sending system SHOULD transmit the
            //   form that uses the minimum quoting possible.

            contextPrior = context;
            context      = contextStack.takeLast();

            switch (context) {
            case ContextComment:
                break;
            case ContextQuotedString:
                parseLocalPart += QLatin1Char('\\');
                parseLocalPart += token;
                atomListLocalPart[elementCount] += QLatin1Char('\\');
                atomListLocalPart[elementCount] += token;
                elementLen += 2; // The maximum sizes specified by RFC 5321 are octet counts, so we
                                 // must include the backslash
                break;
            case ComponentLiteral:
                parseDomain += QLatin1Char('\\');
                parseDomain += token;
                atomListDomain[elementCount] += QLatin1Char('\\');
                atomListDomain[elementCount] += token;
                elementLen += 2; // The maximum sizes specified by RFC 5321 are octet counts, so we
                                 // must include the backslash
                break;
            default:
                returnStatus.push_back(ValidatorEmail::ErrorFatal);
                qCCritical(C_VALIDATOR)
                    << "ValidatorEmail: Quoted pair logic invoked in an invalid context.";
                break;
            }
        } break;
        //-------------------------------------------------------------
        // Comment
        //-------------------------------------------------------------
        case ContextComment:
        {
            // https://tools.ietf.org/html/rfc5322#section-3.2.2
            //   comment         =   "(" *([FWS] ccontent) [FWS] ")"
            //
            //   ccontent        =   ctext / quoted-pair / comment
            if (token == QLatin1Char('(')) { // netsted comment
                // nested comments are OK
                contextStack.push_back(context);
                context = ContextComment;
            } else if (token == QLatin1Char(')')) {
                contextPrior = context;
                context      = contextStack.takeLast();

                // https://tools.ietf.org/html/rfc5322#section-3.2.2
                //   Runs of FWS, comment, or CFWS that occur between lexical tokens in a
                //   structured header field are semantically interpreted as a single
                //   space character.
                //
                // is_email() author's note: This *cannot* mean that we must add a
                // space to the address wherever CFWS appears. This would result in
                // any addr-spec that had CFWS outside a quoted string being invalid
                // for RFC 5321.
                //				if (($context === ISEMAIL_COMPONENT_LOCALPART) ||
                //($context === ISEMAIL_COMPONENT_DOMAIN)) {
                //					$parsedata[$context]			.=
                // ISEMAIL_STRING_SP;
                // $atomlist[$context][$element_count]
                // .= ISEMAIL_STRING_SP; 					$element_len++;
                //				}
            } else if (token == QLatin1Char('\\')) { // Quoted pair
                contextStack.push_back(context);
                context = ContextQuotedPair;
            } else if ((token == QChar(QChar::CarriageReturn)) || (token == QChar(QChar::Space)) ||
                       (token == QChar(QChar::Tabulation))) { // Folding White Space
                if ((token == QChar(QChar::CarriageReturn)) &&
                    ((++i == rawLength) || (email[i] != QChar(QChar::LineFeed)))) {
                    returnStatus.push_back(ValidatorEmail::ErrorCRnoLF);
                    break;
                }

                returnStatus.push_back(ValidatorEmail::CFWSFWS);
                contextStack.push_back(context);
                context    = ContextFWS;
                tokenPrior = token;
            } else { // ctext
                // https://tools.ietf.org/html/rfc5322#section-3.2.3
                //   ctext           =   %d33-39 /          ; Printable US-ASCII
                //                       %d42-91 /          ;  characters not including
                //                       %d93-126 /         ;  "(", ")", or "\"
                //                       obs-ctext
                //
                //   obs-ctext       =   obs-NO-WS-CTL
                //
                //   obs-NO-WS-CTL   =   %d1-8 /            ; US-ASCII control
                //                       %d11 /             ;  characters that do not
                //                       %d12 /             ;  include the carriage
                //                       %d14-31 /          ;  return, line feed, and
                //                       %d127              ;  white space characters

                const ushort uni = token.unicode();

                if ((uni > ValidatorEmailPrivate::asciiEnd) || (uni == 0) ||
                    (uni == ValidatorEmailPrivate::asciiLF)) {
                    returnStatus.push_back(ValidatorEmail::ErrorExpectingCText); // Fatal error
                    break;
                } else if ((uni < ValidatorRulePrivate::asciiSpace) ||
                           (uni == ValidatorEmailPrivate::asciiEnd)) {
                    returnStatus.push_back(ValidatorEmail::DeprecatedCText);
                }
            }
        } break;
        //-------------------------------------------------------------
        // Folding White Space
        //-------------------------------------------------------------
        case ContextFWS:
        {
            // https://tools.ietf.org/html/rfc5322#section-3.2.2
            //   FWS             =   ([*WSP CRLF] 1*WSP) /  obs-FWS
            //                                          ; Folding white space
            // But note the erratum:
            // https://www.rfc-editor.org/errata_search.php?rfc=5322&eid=1908:
            //   In the obsolete syntax, any amount of folding white space MAY be
            //   inserted where the obs-FWS rule is allowed.  This creates the
            //   possibility of having two consecutive "folds" in a line, and
            //   therefore the possibility that a line which makes up a folded header
            //   field could be composed entirely of white space.
            //
            //   obs-FWS         =   1*([CRLF] WSP)
            if (tokenPrior == QChar(QChar::CarriageReturn)) {
                if (token == QChar(QChar::CarriageReturn)) {
                    returnStatus.push_back(ValidatorEmail::ErrorFWSCRLFx2); // Fatal error
                    break;
                }

                if (crlf_count > 0) {
                    if (++crlf_count > 1) {
                        returnStatus.push_back(
                            ValidatorEmail::DeprecatedFWS); // Multiple folds = obsolete FWS
                    }
                } else {
                    crlf_count = 1;
                }
            }

            if (token == QChar(QChar::CarriageReturn)) {
                if ((++i == rawLength) || (email[i] != QChar(QChar::LineFeed))) {
                    returnStatus.push_back(ValidatorEmail::ErrorCRnoLF);
                    break;
                }
            } else if ((token != QChar(QChar::Space)) && (token != QChar(QChar::Tabulation))) {
                if (tokenPrior == QChar(QChar::CarriageReturn)) {
                    returnStatus.push_back(ValidatorEmail::ErrorFWSCRLFEnd); // Fatal error
                    break;
                }

                if (crlf_count > 0) {
                    crlf_count = 0;
                }

                contextPrior = context;
                context      = contextStack.takeLast(); // End of FWS

                // https://tools.ietf.org/html/rfc5322#section-3.2.2
                //   Runs of FWS, comment, or CFWS that occur between lexical tokens in a
                //   structured header field are semantically interpreted as a single
                //   space character.
                //
                // is_email() author's note: This *cannot* mean that we must add a
                // space to the address wherever CFWS appears. This would result in
                // any addr-spec that had CFWS outside a quoted string being invalid
                // for RFC 5321.
                //				if (($context === ISEMAIL_COMPONENT_LOCALPART) ||
                //($context === ISEMAIL_COMPONENT_DOMAIN)) {
                //					$parsedata[$context]			.=
                // ISEMAIL_STRING_SP;
                // $atomlist[$context][$element_count]
                // .= ISEMAIL_STRING_SP; 					$element_len++;
                //				}

                i--; // Look at this token again in the parent context
            }

            tokenPrior = token;
        } break;
        default:
            returnStatus.push_back(ValidatorEmail::ErrorFatal);
            qCCritical(C_VALIDATOR) << "ValidatorEmail: Unknown context";
            break;
        }

        if (static_cast<int>(
                *std::max_element(returnStatus.constBegin(), returnStatus.constEnd())) >
            static_cast<int>(ValidatorEmail::RFC5322)) {
            break;
        }
    }

    // Some simple final tests
    if (static_cast<int>(*std::max_element(returnStatus.constBegin(), returnStatus.constEnd())) <
        static_cast<int>(ValidatorEmail::RFC5322)) {
        if (context == ContextQuotedString) {
            returnStatus.push_back(ValidatorEmail::ErrorUnclosedQuotedStr);
        } else if (context == ContextQuotedPair) {
            returnStatus.push_back(ValidatorEmail::ErrorBackslashEnd);
        } else if (context == ContextComment) {
            returnStatus.push_back(ValidatorEmail::ErrorUnclosedComment);
        } else if (context == ComponentLiteral) {
            returnStatus.push_back(ValidatorEmail::ErrorUnclosedDomLiteral);
        } else if (token == QChar(QChar::CarriageReturn)) {
            returnStatus.push_back(ValidatorEmail::ErrorFWSCRLFEnd);
        } else if (parseDomain.isEmpty()) {
            returnStatus.push_back(ValidatorEmail::ErrorNoDomain);
        } else if (elementLen == 0) {
            returnStatus.push_back(ValidatorEmail::ErrorDotEnd);
        } else if (hypenFlag) {
            returnStatus.push_back(ValidatorEmail::ErrorDomainHyphenEnd);
        } else if (parseDomain.size() > ValidatorEmailPrivate::maxDomainLength) {
            // https://tools.ietf.org/html/rfc5321#section-4.5.3.1.2
            //   The maximum total length of a domain name or number is 255 octets.
            returnStatus.push_back(ValidatorEmail::RFC5322DomainTooLong);
        } else if ((parseLocalPart.size() + 1 + parseDomain.size()) >
                   ValidatorEmailPrivate::maxMailboxLength) {
            // https://tools.ietf.org/html/rfc5321#section-4.1.2
            //   Forward-path   = Path
            //
            //   Path           = "<" [ A-d-l ":" ] Mailbox ">"
            //
            // https://tools.ietf.org/html/rfc5321#section-4.5.3.1.3
            //   The maximum total length of a reverse-path or forward-path is 256
            //   octets (including the punctuation and element separators).
            //
            // Thus, even without (obsolete) routing information, the Mailbox can
            // only be 254 characters long. This is confirmed by this verified
            // erratum to RFC 3696:
            //
            // https://www.rfc-editor.org/errata_search.php?rfc=3696&eid=1690
            //   However, there is a restriction in RFC 2821 on the length of an
            //   address in MAIL and RCPT commands of 254 characters.  Since addresses
            //   that do not fit in those fields are not normally useful, the upper
            //   limit on address lengths should normally be considered to be 254.
            returnStatus.push_back(ValidatorEmail::RFC5322TooLong);
        } else if (elementLen > ValidatorEmailPrivate::maxDnsLabelLength) {
            returnStatus.push_back(ValidatorEmail::RFC5322LabelTooLong);
        }
    }

    // Check DNS?
    bool dnsChecked = false;

    if (checkDns &&
        (static_cast<int>(*std::max_element(returnStatus.constBegin(), returnStatus.constEnd())) <
         static_cast<int>(threshold))) {
        // https://tools.ietf.org/html/rfc5321#section-2.3.5
        //   Names that can
        //   be resolved to MX RRs or address (i.e., A or AAAA) RRs (as discussed
        //   in Section 5) are permitted, as are CNAME RRs whose targets can be
        //   resolved, in turn, to MX or address RRs.
        //
        // https://tools.ietf.org/html/rfc5321#section-5.1
        //   The lookup first attempts to locate an MX record associated with the
        //   name.  If a CNAME record is found, the resulting name is processed as
        //   if it were the initial name. ... If an empty list of MXs is returned,
        //   the address is treated as if it was associated with an implicit MX
        //   RR, with a preference of 0, pointing to that host.

        if (elementCount == 0) {
            parseDomain += QLatin1Char('.');
        }

        QDnsLookup mxLookup(QDnsLookup::MX, parseDomain);
        QEventLoop mxLoop;
        QObject::connect(&mxLookup, &QDnsLookup::finished, &mxLoop, &QEventLoop::quit);
        QTimer::singleShot(ValidatorEmailPrivate::dnsLookupTimeout, &mxLookup, &QDnsLookup::abort);
        mxLookup.lookup();
        mxLoop.exec();

        if ((mxLookup.error() == QDnsLookup::NoError) && !mxLookup.mailExchangeRecords().empty()) {
            dnsChecked = true;
        } else {
            returnStatus.push_back(ValidatorEmail::DnsWarnNoMxRecord);
            QDnsLookup aLookup(QDnsLookup::A, parseDomain);
            QEventLoop aLoop;
            QObject::connect(&aLookup, &QDnsLookup::finished, &aLoop, &QEventLoop::quit);
            QTimer::singleShot(
                ValidatorEmailPrivate::dnsLookupTimeout, &aLookup, &QDnsLookup::abort);
            aLookup.lookup();
            aLoop.exec();

            if ((aLookup.error() == QDnsLookup::NoError) && !aLookup.hostAddressRecords().empty()) {
                dnsChecked = true;
            } else {
                returnStatus.push_back(ValidatorEmail::DnsWarnNoRecord);
            }
        }
    }

    // Check for TLD addresses
    // -----------------------
    // TLD addresses are specifically allowed in RFC 5321 but they are
    // unusual to say the least. We will allocate a separate
    // status to these addresses on the basis that they are more likely
    // to be typos than genuine addresses (unless we've already
    // established that the domain does have an MX record)
    //
    // https://tools.ietf.org/html/rfc5321#section-2.3.5
    //   In the case
    //   of a top-level domain used by itself in an email address, a single
    //   string is used without any dots.  This makes the requirement,
    //   described in more detail below, that only fully-qualified domain
    //   names appear in SMTP transactions on the public Internet,
    //   particularly important where top-level domains are involved.
    //
    // TLD format
    // ----------
    // The format of TLDs has changed a number of times. The standards
    // used by IANA have been largely ignored by ICANN, leading to
    // confusion over the standards being followed. These are not defined
    // anywhere, except as a general component of a DNS host name (a label).
    // However, this could potentially lead to 123.123.123.123 being a
    // valid DNS name (rather than an IP address) and thereby creating
    // an ambiguity. The most authoritative statement on TLD formats that
    // the author can find is in a (rejected!) erratum to RFC 1123
    // submitted by John Klensin, the author of RFC 5321:
    //
    // https://www.rfc-editor.org/errata_search.php?rfc=1123&eid=1353
    //   However, a valid host name can never have the dotted-decimal
    //   form #.#.#.#, since this change does not permit the highest-level
    //   component label to start with a digit even if it is not all-numeric.
    if (!dnsChecked &&
        (static_cast<int>(*std::max_element(returnStatus.constBegin(), returnStatus.constEnd())) <
         static_cast<int>(ValidatorEmail::DNSWarn))) {
        if (elementCount == 0) {
            returnStatus.push_back(ValidatorEmail::RFC5321TLD);
        }

        if (QStringLiteral("0123456789").contains(atomListDomain[elementCount][0])) {
            returnStatus.push_back(ValidatorEmail::RFC5321TLDNumeric);
        }
    }

    if (returnStatus.size() != 1) {
        QList<ValidatorEmail::Diagnose> _rs;
        for (const ValidatorEmail::Diagnose dia : std::as_const(returnStatus)) {
            if (!_rs.contains(dia) && (dia != ValidatorEmail::ValidAddress)) {
                _rs.append(dia); // clazy:exclude=reserve-candidates
            }
        }
        returnStatus = _rs;

        std::sort(returnStatus.begin(), returnStatus.end(), std::greater<>());
    }

    const ValidatorEmail::Diagnose finalStatus = returnStatus.at(0);

    if (diagnoseStruct) {
        diagnoseStruct->finalStatus  = finalStatus;
        diagnoseStruct->returnStatus = returnStatus;
        diagnoseStruct->localpart    = parseLocalPart;
        diagnoseStruct->domain       = parseDomain;
        diagnoseStruct->literal      = parseLiteral;
    }

    return static_cast<int>(finalStatus) < static_cast<int>(threshold);
}

QString ValidatorEmail::diagnoseString(Context *c, Diagnose diagnose, const QString &label)
{
    if (label.isEmpty()) {
        switch (diagnose) {
        case ValidAddress:
            //% "Address is valid. Please note that this does not mean that both the "
            //% "address and the domain actually exist. This address could be issued "
            //% "by the domain owner without breaking the rules of any RFCs."
            return c->qtTrId("cutelyst-valemail-diag-valid");
        case DnsWarnNoMxRecord:
            //% "Could not find an MX record for this address’ domain but an A record exists."
            return c->qtTrId("cutelyst-valemail-diag-nomx");
        case DnsWarnNoRecord:
            //% "Could neither find an MX record nor an A record for this address’ domain."
            return c->qtTrId("cutelyst-valemail-diag-noarec");
        case RFC5321TLD:
            //% "Address is valid but at a Top Level Domain."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321tld");
        case RFC5321TLDNumeric:
            //% "Address is valid but the Top Level Domain begins with a number."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321tldnumeric");
        case RFC5321QuotedString:
            //% "Address is valid but contains a quoted string."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321quotedstring");
        case RFC5321AddressLiteral:
            //% "Address is valid but uses an IP address instead of a domain name."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321addressliteral");
        case RFC5321IPv6Deprecated:
            //% "Address is valid but uses an IP address that contains a :: only "
            //% "eliding one zero group. All implementations must accept and be "
            //% "able to handle any legitimate RFC 4291 format."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321ipv6deprecated");
        case CFWSComment:
            //% "Address contains comments."
            return c->qtTrId("cutelyst-valemail-diag-cfwscomment");
        case CFWSFWS:
            //% "Address contains folding white spaces like line breaks."
            return c->qtTrId("cutelyst-valemail-diag-cfwsfws");
        case DeprecatedLocalpart:
            //% "The local part is in a deprecated form."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedlocalpart");
        case DeprecatedFWS:
            //% "Address contains an obsolete form of folding white spaces."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedfws");
        case DeprecatedQText:
            //% "A quoted string contains a deprecated character."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedqtext");
        case DeprecatedQP:
            //% "A quoted pair contains a deprecated character."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedqp");
        case DeprecatedComment:
            //% "Address contains a comment in a position that is deprecated."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedcomment");
        case DeprecatedCText:
            //% "A comment contains a deprecated character."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedctext");
        case DeprecatedCFWSNearAt:
            //% "Address contains a comment or folding white space around the @ sign."
            return c->qtTrId("cutelyst-valemail-diag-cfwsnearat");
        case RFC5322Domain:
            //% "Address is RFC 5322 compliant but contains domain characters that "
            //% "are not allowed by DNS."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domain");
        case RFC5322TooLong:
            //% "The address exceeds the maximum allowed length of %1 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322toolong")
                .arg(c->locale().toString(ValidatorEmailPrivate::maxMailboxLength));
        case RFC5322LocalTooLong:
            //% "The local part of the address exceeds the maximum allowed length "
            //% "of %1 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322localtoolong")
                .arg(c->locale().toString(ValidatorEmailPrivate::maxLocalPartLength));
        case RFC5322DomainTooLong:
            //% "The domain part exceeds the maximum allowed length of %1 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domaintoolong")
                .arg(c->locale().toString(ValidatorEmailPrivate::maxDomainLength));
        case RFC5322LabelTooLong:
            //% "One of the labels/sections in the domain part exceeds the maximum allowed "
            //% "length of %1 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322labeltoolong")
                .arg(c->locale().toString(ValidatorEmailPrivate::maxDnsLabelLength));
        case RFC5322DomainLiteral:
            //% "The domain literal is not a valid RFC 5321 address literal."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domainliteral");
        case RFC5322DomLitOBSDText:
            //% "The domain literal is not a valid RFC 5321 domain literal and it "
            //% "contains obsolete characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domlitobsdtext");
        case RFC5322IPv6GroupCount:
            //% "The IPv6 literal address contains the wrong number of groups."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6groupcount");
        case RFC5322IPv62x2xColon:
            //% "The IPv6 literal address contains too many :: sequences."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv62x2xcolon");
        case RFC5322IPv6BadChar:
            //% "The IPv6 address contains an illegal group of characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6badchar");
        case RFC5322IPv6MaxGroups:
            //% "The IPv6 address has too many groups."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6maxgroups");
        case RFC5322IPv6ColonStart:
            //% "The IPv6 address starts with a single colon."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6colonstart");
        case RFC5322IPv6ColonEnd:
            //% "The IPv6 address ends with a single colon."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6colonend");
        case ErrorExpectingDText:
            //% "A domain literal contains a character that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingdtext");
        case ErrorNoLocalPart:
            //% "Address has no local part."
            return c->qtTrId("cutelyst-valemail-diag-errnolocalpart");
        case ErrorNoDomain:
            //% "Address has no domain part."
            return c->qtTrId("cutelyst-valemail-diag-errnodomain");
        case ErrorConsecutiveDots:
            //% "The address must not contain consecutive dots."
            return c->qtTrId("cutelyst-valemail-diag-errconsecutivedots");
        case ErrorATextAfterCFWS:
            //% "Address contains text after a comment or folding white space."
            return c->qtTrId("cutelyst-valemail-diag-erratextaftercfws");
        case ErrorATextAfterQS:
            //% "Address contains text after a quoted string."
            return c->qtTrId("cutelyst-valemail-diag-erratextafterqs");
        case ErrorATextAfterDomLit:
            //% "Extra characters were found after the end of the domain literal."
            return c->qtTrId("cutelyst-valemail-diag-erratextafterdomlit");
        case ErrorExpectingQpair:
            //% "The Address contains a character that is not allowed in a quoted pair."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingqpair");
        case ErrorExpectingAText:
            //% "Address contains a character that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingatext");
        case ErrorExpectingQText:
            //% "A quoted string contains a character that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingqtext");
        case ErrorExpectingCText:
            //% "A comment contains a character that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingctext");
        case ErrorBackslashEnd:
            //% "The address can not end with a backslash."
            return c->qtTrId("cutelyst-valemail-diag-errbackslashend");
        case ErrorDotStart:
            //% "Neither part of the address may begin with a dot."
            return c->qtTrId("cutelyst-valemail-diag-errdotstart");
        case ErrorDotEnd:
            //% "Neither part of the address may end with a dot."
            return c->qtTrId("cutelyst-valemail-diag-errdotend");
        case ErrorDomainHyphenStart:
            //% "A domain or subdomain can not begin with a hyphen."
            return c->qtTrId("cutelyst-valemail-diag-errdomainhyphenstart");
        case ErrorDomainHyphenEnd:
            //% "A domain or subdomain can not end with a hyphen."
            return c->qtTrId("cutelyst-valemail-diag-errdomainhyphenend");
        case ErrorUnclosedQuotedStr:
            //% "Unclosed quoted string. (Missing double quotation mark)"
            return c->qtTrId("cutelyst-valemail-diag-errunclosedquotedstr");
        case ErrorUnclosedComment:
            //% "Unclosed comment. (Missing closing parentheses)"
            return c->qtTrId("cutelyst-valemail-diag-errunclosedcomment");
        case ErrorUnclosedDomLiteral:
            //% "Domain literal is missing its closing bracket."
            return c->qtTrId("cutelyst-valemail-diag-erruncloseddomliteral");
        case ErrorFWSCRLFx2:
            //% "Folding white space contains consecutive line break sequences (CRLF)."
            return c->qtTrId("cutelyst-valemail-diag-errfwscrlfx2");
        case ErrorFWSCRLFEnd:
            //% "Folding white space ends with a line break sequence (CRLF)."
            return c->qtTrId("cutelyst-valemail-diag-errfwscrlfend");
        case ErrorCRnoLF:
            //% "Address contains a carriage return (CR) that is not followed by a "
            //% "line feed (LF)."
            return c->qtTrId("cutelyst-valemail-diag-errcrnolf");
        case ErrorFatal:
            //% "A fatal error occurred while parsing the address."
            return c->qtTrId("cutelyst-valemail-diag-errfatal");
        default:
            return {};
        }

    } else {

        switch (diagnose) {
        case ValidAddress:
            //% "The address in the “%1” field is valid. Please note that this does not mean "
            //% "that both the address and the domain actually exist. This address could be "
            //% "issued by the domain owner without breaking the rules of any RFCs."
            return c->qtTrId("cutelyst-valemail-diag-valid-label").arg(label);
        case DnsWarnNoMxRecord:
            //% "Could not find an MX record for the address’ domain in the “%1” "
            //% "field but an A record exists."
            return c->qtTrId("cutelyst-valemail-diag-nomx-label").arg(label);
        case DnsWarnNoRecord:
            //% "Could neither find an MX record nor an A record for the address’ "
            //% "domain in the “%1” field."
            return c->qtTrId("cutelyst-valemail-diag-noarec-label").arg(label);
        case RFC5321TLD:
            //% "The address in the “%1” field is valid but at a Top Level Domain."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321tld-label").arg(label);
        case RFC5321TLDNumeric:
            //% "The address in the “%1” field is valid but the Top Level Domain "
            //% "begins with a number."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321tldnumeric-label").arg(label);
        case RFC5321QuotedString:
            //% "The address in the “%1” field is valid but contains a quoted string."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321quotedstring-label").arg(label);
        case RFC5321AddressLiteral:
            //% "The address in the “%1” field is valid but uses an IP address "
            //% "instead of a domain name."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321addressliteral-label").arg(label);
        case RFC5321IPv6Deprecated:
            //% "The address in the “%1” field is valid but uses an IP address that "
            //% "contains a :: only eliding one zero group. All implementations "
            //% "must accept and be able to handle any legitimate RFC 4291 format."
            return c->qtTrId("cutelyst-valemail-diag-rfc5321ipv6deprecated-label").arg(label);
        case CFWSComment:
            //% "The address in the “%1” field contains comments."
            return c->qtTrId("cutelyst-valemail-diag-cfwscomment-label").arg(label);
        case CFWSFWS:
            //% "The address in the “%1” field contains folding white spaces like "
            //% "line breaks."
            return c->qtTrId("cutelyst-valemail-diag-cfwsfws-label").arg(label);
        case DeprecatedLocalpart:
            //% "The local part of the address in the “%1” field is in a deprecated form."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedlocalpart-label").arg(label);
        case DeprecatedFWS:
            //% "The address in the “%1” field contains an obsolete form of folding "
            //% "white spaces."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedfws-label").arg(label);
        case DeprecatedQText:
            //% "A quoted string in the address in the “%1” field contains a "
            //% "deprecated character."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedqtext-label").arg(label);
        case DeprecatedQP:
            //% "A quoted pair in the address in the “%1” field contains a "
            //% "deprecate character."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedqp-label").arg(label);
        case DeprecatedComment:
            //% "The address in the “%1” field contains a comment in a position "
            //% "that is deprecated."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedcomment-label").arg(label);
        case DeprecatedCText:
            //% "A comment in the address in the “%1” field contains a deprecated character."
            return c->qtTrId("cutelyst-valemail-diag-deprecatedctext-label").arg(label);
        case DeprecatedCFWSNearAt:
            //% "The address in the “%1” field contains a comment or folding white "
            //% "space around the @ sign."
            return c->qtTrId("cutelyst-valemail-diag-cfwsnearat-label").arg(label);
        case RFC5322Domain:
            //% "The address in the “%1” field is RFC 5322 compliant but contains "
            //% "domain characters that are not allowed by DNS."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domain-label").arg(label);
        case RFC5322TooLong:
            //% "The address in the “%1” field exceeds the maximum allowed length "
            //% "of %2 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322toolong-label")
                .arg(label, c->locale().toString(ValidatorEmailPrivate::maxMailboxLength));
        case RFC5322LocalTooLong:
            //% "The local part of the address in the “%1” field exceeds the maximum allowed "
            //% "length of %2 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322localtoolong-label")
                .arg(label, c->locale().toString(ValidatorEmailPrivate::maxLocalPartLength));
        case RFC5322DomainTooLong:
            //% "The domain part of the address in the “%1” field exceeds the maximum "
            //% "allowed length of %2 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domaintoolong-label")
                .arg(label, c->locale().toString(ValidatorEmailPrivate::maxDomainLength));
        case RFC5322LabelTooLong:
            //% "The domain part of the address in the “%1” field contains an element/section "
            //% "that exceeds the maximum allowed lenght of %2 characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322labeltoolong-label")
                .arg(label, c->locale().toString(ValidatorEmailPrivate::maxDnsLabelLength));
        case RFC5322DomainLiteral:
            //% "The domain literal of the address in the “%1” field is not a valid "
            //% "RFC 5321 address literal."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domainliteral-label").arg(label);
        case RFC5322DomLitOBSDText:
            //% "The domain literal of the address in the “%1” field is not a valid "
            //% "RFC 5321 domain literal and it contains obsolete characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322domlitobsdtext-label").arg(label);
        case RFC5322IPv6GroupCount:
            //% "The IPv6 literal of the address in the “%1” field contains the "
            //% "wrong number of groups."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6groupcount-label").arg(label);
        case RFC5322IPv62x2xColon:
            //% "The IPv6 literal of the address in the “%1” field contains too "
            //% "many :: sequences."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv62x2xcolon-label").arg(label);
        case RFC5322IPv6BadChar:
            //% "The IPv6 address of the email address in the “%1” field contains "
            //% "an illegal group of characters."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6badchar-label").arg(label);
        case RFC5322IPv6MaxGroups:
            //% "The IPv6 address of the email address in the “%1” field has too many groups."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6maxgroups-label").arg(label);
        case RFC5322IPv6ColonStart:
            //% "The IPv6 address of the email address in the “%1” field starts "
            //% "with a single colon."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6colonstart-label").arg(label);
        case RFC5322IPv6ColonEnd:
            //% "The IPv6 address of the email address in the “%1” field ends with "
            //% "a single colon."
            return c->qtTrId("cutelyst-valemail-diag-rfc5322ipv6colonend-label").arg(label);
        case ErrorExpectingDText:
            //% "A domain literal of the address in the “%1” field contains a "
            //% "character that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingdtext-label").arg(label);
        case ErrorNoLocalPart:
            //% "The address in the “%1” field has no local part."
            return c->qtTrId("cutelyst-valemail-diag-errnolocalpart-label").arg(label);
        case ErrorNoDomain:
            //% "The address in the “%1” field has no domain part."
            return c->qtTrId("cutelyst-valemail-diag-errnodomain-label").arg(label);
        case ErrorConsecutiveDots:
            //% "The address in the “%1” field must not contain consecutive dots."
            return c->qtTrId("cutelyst-valemail-diag-errconsecutivedots-label").arg(label);
        case ErrorATextAfterCFWS:
            //% "The address in the “%1” field contains text after a comment or "
            //% "folding white space."
            return c->qtTrId("cutelyst-valemail-diag-erratextaftercfws-label").arg(label);
        case ErrorATextAfterQS:
            //% "The address in the “%1” field contains text after a quoted string."
            return c->qtTrId("cutelyst-valemail-diag-erratextafterqs-label").arg(label);
        case ErrorATextAfterDomLit:
            //% "Extra characters were found after the end of the domain literal of "
            //% "the address in the “%1” field."
            return c->qtTrId("cutelyst-valemail-diag-erratextafterdomlit-label").arg(label);
        case ErrorExpectingQpair:
            //% "The address in the “%1” field contains a character that is not "
            //% "allowed in a quoted pair."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingqpair-label").arg(label);
        case ErrorExpectingAText:
            //% "The address in the “%1” field contains a character that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingatext-label").arg(label);
        case ErrorExpectingQText:
            //% "A quoted string in the address in the “%1” field contains a "
            //% "character that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingqtext-label").arg(label);
        case ErrorExpectingCText:
            //% "A comment in the address in the “%1” field contains a character "
            //% "that is not allowed."
            return c->qtTrId("cutelyst-valemail-diag-errexpectingctext-label").arg(label);
        case ErrorBackslashEnd:
            //% "The address in the “%1” field can't end with a backslash."
            return c->qtTrId("cutelyst-valemail-diag-errbackslashend-label").arg(label);
        case ErrorDotStart:
            //% "Neither part of the address in the “%1” field may begin with a dot."
            return c->qtTrId("cutelyst-valemail-diag-errdotstart-label").arg(label);
        case ErrorDotEnd:
            //% "Neither part of the address in the “%1” field may end with a dot."
            return c->qtTrId("cutelyst-valemail-diag-errdotend-label").arg(label);
        case ErrorDomainHyphenStart:
            //% "A domain or subdomain of the address in the “%1” field can not "
            //% "begin with a hyphen."
            return c->qtTrId("cutelyst-valemail-diag-errdomainhyphenstart-label").arg(label);
        case ErrorDomainHyphenEnd:
            //% "A domain or subdomain of the address in the “%1” field can not end "
            //% "with a hyphen."
            return c->qtTrId("cutelyst-valemail-diag-errdomainhyphenend-label").arg(label);
        case ErrorUnclosedQuotedStr:
            //% "Unclosed quoted string in the address in the “%1” field. (Missing "
            //% "double quotation mark)"
            return c->qtTrId("cutelyst-valemail-diag-errunclosedquotedstr-label").arg(label);
        case ErrorUnclosedComment:
            //% "Unclosed comment in the address in the “%1” field. (Missing "
            //% "closing parentheses)"
            return c->qtTrId("cutelyst-valemail-diag-errunclosedcomment-label").arg(label);
        case ErrorUnclosedDomLiteral:
            //% "Domain literal of the address in the “%1” field is missing its "
            //% "closing bracket."
            return c->qtTrId("cutelyst-valemail-diag-erruncloseddomliteral-label").arg(label);
        case ErrorFWSCRLFx2:
            //% "Folding white space in the address in the “%1” field contains "
            //% "consecutive line break sequences (CRLF)."
            return c->qtTrId("cutelyst-valemail-diag-errfwscrlfx2-label").arg(label);
        case ErrorFWSCRLFEnd:
            //% "Folding white space in the address in the “%1” field ends with a "
            //% "line break sequence (CRLF)."
            return c->qtTrId("cutelyst-valemail-diag-errfwscrlfend-label").arg(label);
        case ErrorCRnoLF:
            //% "The address in the “%1” field contains a carriage return (CR) that "
            //% "is not followed by a line feed (LF)."
            return c->qtTrId("cutelyst-valemail-diag-errcrnolf-label").arg(label);
        case ErrorFatal:
            //% "A fatal error occurred while parsing the address in the “%1” field."
            return c->qtTrId("cutelyst-valemail-diag-errfatal-label").arg(label);
        default:
            return {};
        }
    }
}

QString ValidatorEmail::categoryString(Context *c, Category category, const QString &label)
{
    if (label.isEmpty()) {
        switch (category) {
        case Valid:
            //% "Address is valid."
            return c->qtTrId("cutelyst-valemail-cat-valid");
        case DNSWarn:
            //% "Address is valid but a DNS check was not successful."
            return c->qtTrId("cutelyst-valemail-cat-dnswarn");
        case RFC5321:
            //% "Address is valid for SMTP but has unusual elements."
            return c->qtTrId("cutelyst-valemail-cat-rfc5321");
        case CFWS:
            //% "Address is valid within the message but can not be used unmodified "
            //% "for the envelope."
            return c->qtTrId("cutelyst-valemail-cat-cfws");
        case Deprecated:
            //% "Address contains deprecated elements but may still be valid in "
            //% "restricted contexts."
            return c->qtTrId("cutelyst-valemail-cat-deprecated");
        case RFC5322:
            //% "The address is only valid according to the broad definition of RFC "
            //% "5322. It is otherwise invalid."
            return c->qtTrId("cutelyst-valemail-cat-rfc5322");
        default:
            //% "Address is invalid for any purpose."
            return c->qtTrId("cutelyst-valemail-cat-invalid");
        }
    } else {
        switch (category) {
        case Valid:
            //% "The address in the “%1” field is valid."
            return c->qtTrId("cutelyst-valemail-cat-valid-label").arg(label);
        case DNSWarn:
            //% "The address in the “%1” field is valid but a DNS check was not successful."
            return c->qtTrId("cutelyst-valemail-cat-dnswarn-label").arg(label);
        case RFC5321:
            //% "The address in the “%1” field is valid for SMTP but has unusual elements."
            return c->qtTrId("cutelyst-valemail-cat-rfc5321-label").arg(label);
        case CFWS:
            //% "The address in the “%1” field is valid within the message but can "
            //% "not be used unmodified for the envelope."
            return c->qtTrId("cutelyst-valemail-cat-cfws-label").arg(label);
        case Deprecated:
            //% "The address in the “%1” field contains deprecated elements but may "
            //% "still be valid in restricted contexts."
            return c->qtTrId("cutelyst-valemail-cat-deprecated-label").arg(label);
        case RFC5322:
            //% "The address in the “%1” field is only valid according to the broad "
            //% "definition of RFC 5322. It is otherwise invalid."
            return c->qtTrId("cutelyst-valemail-cat-rfc5322-label").arg(label);
        default:
            //% "The address in the “%1” field is invalid for any purpose."
            return c->qtTrId("cutelyst-valemail-cat-invalid-label").arg(label);
        }
    }
}

Cutelyst::ValidatorEmail::Category ValidatorEmail::category(Diagnose diagnose)
{
    Category cat = Error;

    const auto diag = static_cast<int>(diagnose);

    if (diag < static_cast<int>(Valid)) {
        cat = Valid;
    } else if (diag < static_cast<int>(DNSWarn)) {
        cat = DNSWarn;
    } else if (diag < static_cast<int>(RFC5321)) {
        cat = RFC5321;
    } else if (diag < static_cast<int>(CFWS)) {
        cat = CFWS;
    } else if (diag < static_cast<int>(Deprecated)) {
        cat = Deprecated;
    } else if (diag < static_cast<int>(RFC5322)) {
        cat = RFC5322;
    }

    return cat;
}

QString ValidatorEmail::categoryString(Context *c, Diagnose diagnose, const QString &label)
{
    return categoryString(c, category(diagnose), label);
}

bool ValidatorEmail::validate(const QString &email,
                              Category threshold,
                              Options options,
                              QList<Cutelyst::ValidatorEmail::Diagnose> *diagnoses)
{
    ValidatorEmailDiagnoseStruct diag;
    bool ret = ValidatorEmailPrivate::checkEmail(email, options, threshold, &diag);

    if (diagnoses) {
        *diagnoses = diag.returnStatus;
    }

    return ret;
}

#include "moc_validatoremail.cpp"
