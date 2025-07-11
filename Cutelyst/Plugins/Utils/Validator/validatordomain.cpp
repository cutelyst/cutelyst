/*
 * SPDX-FileCopyrightText: (C) 2018-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordomain_p.h"

#include <QDnsLookup>
#include <QEventLoop>
#include <QHostAddress>
#include <QStringList>
#include <QTimer>
#include <QUrl>

using namespace Cutelyst;

ValidatorDomain::ValidatorDomain(const QString &field,
                                 Options options,
                                 const ValidatorMessages &messages,
                                 const QString &defValKey)
    : ValidatorRule(*new ValidatorDomainPrivate(field, options, messages, defValKey))
{
}

ValidatorDomain::~ValidatorDomain() = default;

bool ValidatorDomain::validate(const QString &value,
                               Cutelyst::ValidatorDomain::Diagnose *diagnose,
                               QString *extractedValue)
{
    Diagnose diag = Valid;

    const bool hasRootDot        = value.endsWith(u'.');
    const QString withoutRootDot = hasRootDot ? value.chopped(1) : value;

    // convert to lower case puny code
    const QString ace = QString::fromLatin1(QUrl::toAce(withoutRootDot)).toLower();

    // split up the utf8 string into parts to get the non puny code TLD
    const QStringList nonAceParts = withoutRootDot.split(u'.');
    if (!nonAceParts.empty()) {
        const QString tld = nonAceParts.last();
        if (!tld.isEmpty()) {
            // there are no TLDs with digits inside, but IDN TLDs can
            // have digits in their puny code representation, so we have
            // to check at first if the IDN TLD contains digits before
            // checking the ACE puny code
            for (const QChar &ch : tld) {
                const char16_t uc = ch.unicode();
                if (((uc >= ValidatorRulePrivate::ascii_0) &&
                     (uc <= ValidatorRulePrivate::ascii_9)) ||
                    (uc == ValidatorRulePrivate::ascii_dash)) {
                    diag = InvalidTLD;
                    break;
                }
            }

            if (diag == Valid) {
                if (!ace.isEmpty()) {
                    // maximum length of the name in the DNS is 253 without the last dot
                    if (ace.length() <= ValidatorDomainPrivate::maxDnsNameWithLastDot) {
                        const QStringList parts = ace.split(u'.', Qt::KeepEmptyParts);
                        // there has to be more than only the TLD
                        if (parts.size() > 1) {
                            // the TLD can not have only 1 char
                            if (parts.last().length() > 1) {
                                for (int i = 0; i < parts.size(); ++i) {

                                    if (diag != Valid) {
                                        break;
                                    }

                                    const QString &part = parts.at(i);

                                    if (part.isEmpty()) {
                                        diag = EmptyLabel;
                                        break;
                                    }

                                    // labels/parts can have a maximum length of 63 chars
                                    if (part.length() > ValidatorDomainPrivate::maxDnsLabelLength) {
                                        diag = LabelTooLong;
                                        break;
                                    }

                                    const bool isTld        = (i == (parts.size() - 1));
                                    const bool isPunyCode   = part.startsWith(u"xn--");
                                    const qsizetype partEnd = part.size() - 1;

                                    for (int j = 0; j < part.size(); ++j) {
                                        const char16_t uc = part.at(j).unicode();
                                        const bool isDigit =
                                            ((uc >= ValidatorRulePrivate::ascii_0) &&
                                             (uc <= ValidatorRulePrivate::ascii_9));
                                        const bool isDash =
                                            (uc == ValidatorRulePrivate::ascii_dash);
                                        // no part/label can start with a digit or a
                                        // dash
                                        if (j == 0 && (isDash || isDigit)) {
                                            diag = isDash ? DashStart : DigitStart;
                                            break;
                                        }
                                        // no part/label can end with a dash
                                        if (j == partEnd && isDash) {
                                            diag = DashEnd;
                                            break;
                                        }
                                        const bool isChar =
                                            ((uc >= ValidatorRulePrivate::ascii_a) &&
                                             (uc <= ValidatorRulePrivate::ascii_z));
                                        if (!isTld) {
                                            // if it is not the tld, it can have a-z 0-9
                                            // and -
                                            if (!(isDigit || isDash || isChar)) {
                                                diag = InvalidChars;
                                                break;
                                            }
                                        } else {
                                            if (isPunyCode) {
                                                if (!(isDigit || isDash || isChar)) {
                                                    diag = InvalidTLD;
                                                    break;
                                                }
                                            } else {
                                                if (!isChar) {
                                                    diag = InvalidTLD;
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                            } else {
                                diag = InvalidTLD;
                            }
                        } else {
                            diag = InvalidLabelCount;
                        }
                    } else {
                        diag = TooLong;
                    }
                } else {
                    diag = EmptyLabel;
                }
            }
        } else {
            diag = EmptyLabel;
        }
    } else {
        diag = EmptyLabel;
    }

    if (diagnose) {
        *diagnose = diag;
    }

    if (diag == Valid && extractedValue) {
        if (hasRootDot) {
            *extractedValue = ace + u'.';
        } else {
            *extractedValue = ace;
        }
    }

    return diag == Valid;
}

void ValidatorDomain::validateCb(
    const QString &value,
    Options options,
    std::function<void(Diagnose diagnose, const QString &extractedValue)> cb)
{
    Diagnose diag;
    QString extracted;

    bool isValid = ValidatorDomain::validate(value, &diag, &extracted);
    if (!isValid) {
        cb(diag, {});
        return;
    }

    if (!options.testAnyFlag(CheckDNS)) {
        cb(diag, extracted);
        return;
    }

    if (options.testFlag(CheckARecord)) {

        auto dns = new QDnsLookup{QDnsLookup::A, extracted};
        QObject::connect(dns, &QDnsLookup::finished, [dns, options, cb, extracted] {
            if (dns->error() == QDnsLookup::NoError) {
                if (dns->hostAddressRecords().empty()) {
                    cb(MissingDNS, extracted);
                } else {
                    if (!options.testFlag(CheckAAAARecord)) {
                        cb(Valid, extracted);
                    } else {

                        auto dns2 = new QDnsLookup{QDnsLookup::AAAA, extracted};
                        QObject::connect(
                            dns2, &QDnsLookup::finished, [dns2, options, cb, extracted] {
                            if (dns2->error() == QDnsLookup::NoError) {
                                if (dns2->hostAddressRecords().empty()) {
                                    cb(MissingDNS, extracted);
                                } else {
                                    cb(Valid, extracted);
                                }
                            } else if (dns2->error() == QDnsLookup::OperationCancelledError) {
                                cb(DNSTimeout, extracted);
                            } else {
                                cb(DNSError, extracted);
                            }
                            dns2->deleteLater();
                        });
                        QTimer::singleShot(
                            ValidatorDomainPrivate::dnsLookupTimeout, dns2, &QDnsLookup::abort);
                        dns2->lookup();
                    }
                }
            } else if (dns->error() == QDnsLookup::OperationCancelledError) {
                cb(DNSTimeout, extracted);
            } else {
                cb(DNSError, extracted);
            }
            dns->deleteLater();
        });
        QTimer::singleShot(ValidatorDomainPrivate::dnsLookupTimeout, dns, &QDnsLookup::abort);
        dns->lookup();

    } else if (options.testFlag(CheckAAAARecord)) {

        auto dns2 = new QDnsLookup{QDnsLookup::AAAA, extracted};
        QObject::connect(dns2, &QDnsLookup::finished, [dns2, options, cb, extracted] {
            if (dns2->error() == QDnsLookup::NoError) {
                if (dns2->hostAddressRecords().empty()) {
                    cb(MissingDNS, extracted);
                } else {
                    cb(Valid, extracted);
                }
            } else if (dns2->error() == QDnsLookup::OperationCancelledError) {
                cb(DNSTimeout, extracted);
            } else {
                cb(DNSError, extracted);
            }
            dns2->deleteLater();
        });
        QTimer::singleShot(ValidatorDomainPrivate::dnsLookupTimeout, dns2, &QDnsLookup::abort);
        dns2->lookup();
    }
}

QString ValidatorDomain::diagnoseString(const Context *c, Diagnose diagnose, const QString &label)
{
    if (label.isEmpty()) {
        switch (diagnose) {
        case MissingDNS:
            //% "The domain name seems to be valid but could not be found in the "
            //% "domain name system."
            return c->qtTrId("cutelyst-valdomain-diag-missingdns");
        case InvalidChars:
            //% "The domain name contains characters that are not allowed."
            return c->qtTrId("cutelyst-valdomain-diag-invalidchars");
        case LabelTooLong:
            //% "At least one of the sections separated by dots exceeds the maximum "
            //% "allowed length of 63 characters. Note that internationalized domain "
            //% "names with non-ASCII characters can be longer internally than they are "
            //% "displayed."
            return c->qtTrId("cutelyst-valdomain-diag-labeltoolong");
        case TooLong:
            //% "The full name of the domain must not be longer than 253 characters. Note that "
            //% "internationalized domain names with non-ASCII character can be longer internally "
            //% "than they are displayed."
            return c->qtTrId("cutelyst-valdomain-diag-toolong");
        case InvalidLabelCount:
            //% "This is not a valid domain name because it has either no parts "
            //% "(is empty) or only has a top level domain."
            return c->qtTrId("cutelyst-valdomain-diag-invalidlabelcount");
        case EmptyLabel:
            //% "At least one of the sections separated by dots is empty. Check "
            //% "whether you have entered two dots consecutively."
            return c->qtTrId("cutelyst-valdomain-diag-emptylabel");
        case InvalidTLD:
            //% "The top level domain (last part) contains characters that are "
            //% "not allowed, like digits and/or dashes."
            return c->qtTrId("cutelyst-valdomain-diag-invalidtld");
        case DashStart:
            //% "Domain name sections are not allowed to start with a dash."
            return c->qtTrId("cutelyst-valdomain-diag-dashstart");
        case DashEnd:
            //% "Domain name sections are not allowed to end with a dash."
            return c->qtTrId("cutelyst-valdomain-diag-dashend");
        case DigitStart:
            //% "Domain name sections are not allowed to start with a digit."
            return c->qtTrId("cutelyst-valdomain-diag-digitstart");
        case Valid:
            //% "The domain name is valid."
            return c->qtTrId("cutelyst-valdomain-diag-valid");
        case DNSTimeout:
            //% "The DNS lookup was aborted because it took too long."
            return c->qtTrId("cutelyst-valdomain-diag-dnstimeout");
        case DNSError:
            //% "An error occured while performing the DNS lookup."
            return c->qtTrId("cutelyst-valdomain-diag-dnserror");
        default:
            Q_ASSERT_X(false, "domain validation diagnose", "invalid diagnose");
            return {};
        }
    } else {
        switch (diagnose) {
        case MissingDNS:
            //% "The domain name in the “%1“ field seems to be valid but could "
            //% "not be found in the domain name system."
            return c->qtTrId("cutelyst-valdomain-diag-missingdns-label").arg(label);
        case InvalidChars:
            //% "The domain name in the “%1“ field contains characters that are not allowed."
            return c->qtTrId("cutelyst-valdomain-diag-invalidchars-label").arg(label);
        case LabelTooLong:
            //% "The domain name in the “%1“ field is not valid because at least "
            //% "one of the sections separated by dots exceeds the maximum "
            //% "allowed length of 63 characters. Note that internationalized "
            //% "domain names with non-ASCII characters can be longer internally "
            //% "than they are displayed."
            return c->qtTrId("cutelyst-valdomain-diag-labeltoolong-label").arg(label);
        case TooLong:
            //% "The full name of the domain in the “%1” field must not be longer "
            //% "than 253 characters. Note that internationalized domain names "
            //% "with non-ASCII characters can be longer internally than they are displayed."
            return c->qtTrId("cutelyst-valdomain-diag-toolong-label").arg(label);
        case InvalidLabelCount:
            //% "The “%1” field does not contain a valid domain name because it "
            //% "has either no parts (is empty) or only has a top level domain."
            return c->qtTrId("cutelyst-valdomain-diag-invalidlabelcount-label").arg(label);
        case EmptyLabel:
            //% "The domain name in the “%1“ field is not valid because at least "
            //% "one of the sections separated by dots is empty. Check whether "
            //% "you have entered two dots consecutively."
            return c->qtTrId("cutelyst-valdomain-diag-emptylabel-label").arg(label);
        case InvalidTLD:
            //% "The top level domain (last part) of the domain name in the “%1” field "
            //% "contains characters that are not allowed, like digits and or dashes."
            return c->qtTrId("cutelyst-valdomain-diag-invalidtld-label").arg(label);
        case DashStart:
            //% "The domain name in the “%1“ field is not valid because domain "
            //% "name sections are not allowed to start with a dash."
            return c->qtTrId("cutelyst-valdomain-diag-dashstart-label").arg(label);
        case DashEnd:
            //% "The domain name in the “%1“ field is not valid because domain "
            //% "name sections are not allowed to end with a dash."
            return c->qtTrId("cutelyst-valdomain-diag-dashend-label").arg(label);
        case DigitStart:
            //% "The domain name in the “%1“ field is not valid because domain "
            //% "name sections are not allowed to start with a digit."
            return c->qtTrId("cutelyst-valdomain-diag-digitstart-label").arg(label);
        case Valid:
            //% "The domain name in the “%1” field is valid."
            return c->qtTrId("cutelyst-valdomain-diag-valid-label").arg(label);
        case DNSTimeout:
            //% "The DNS lookup for the domain name in the “%1” field was aborted "
            //% "because it took too long."
            return c->qtTrId("cutelyst-valdomain-diag-dnstimeout-label").arg(label);
        case DNSError:
            //% "The DNS lookup for the domain name in the “%1” field failed "
            //% "becaus of an error in the DNS resolution."
            return c->qtTrId("cutelyst-valdomain-diag-dnserror-label");
        default:
            Q_ASSERT_X(false, "domain validation diagnose", "invalid diagnose");
            return {};
        }
    }
}

void writeDebugString(const QString &valInfo, ValidatorDomain::Diagnose diag, const QString &v)
{
    switch (diag) {
    case ValidatorDomain::Valid:
        break;
    case ValidatorDomain::MissingDNS:
        qCDebug(C_VALIDATOR).noquote() << valInfo << "Can not find valid DNS entry for" << v;
        break;
    case ValidatorDomain::InvalidChars:
        qCDebug(C_VALIDATOR).noquote()
            << valInfo << "The domain name contains characters that are not allowed";
        break;
    case ValidatorDomain::LabelTooLong:
        qCDebug(C_VALIDATOR).noquote()
            << valInfo << "At least on of the domain name labels exceeds the maximum" << "size of"
            << ValidatorDomainPrivate::maxDnsLabelLength << "characters";
        break;
    case ValidatorDomain::TooLong:
        qCDebug(C_VALIDATOR).noquote()
            << valInfo << "The domain name exceeds the maximum size of"
            << ValidatorDomainPrivate::maxDnsNameWithLastDot << "characters";
        break;
    case ValidatorDomain::InvalidLabelCount:
        qCDebug(C_VALIDATOR).noquote()
            << valInfo << "Invalid label count. Either no labels or only TLD";
        break;
    case ValidatorDomain::EmptyLabel:
        qCDebug(C_VALIDATOR).noquote()
            << valInfo << "At least one of the domain name labels is empty";
        break;
    case ValidatorDomain::InvalidTLD:
        qCDebug(C_VALIDATOR).noquote()
            << valInfo << "The TLD label contains characters that are not allowed";
        break;
    case ValidatorDomain::DashStart:
        qCDebug(C_VALIDATOR).noquote() << valInfo << "At least one label starts with a dash";
        break;
    case ValidatorDomain::DashEnd:
        qCDebug(C_VALIDATOR).noquote() << valInfo << "At least one label ends with a dash";
        break;
    case ValidatorDomain::DigitStart:
        qCDebug(C_VALIDATOR).noquote() << valInfo << "At least one label starts with a digit";
        break;
    case ValidatorDomain::DNSTimeout:
        qCDebug(C_VALIDATOR).noquote() << valInfo << "The DNS lookup exceeds the timeout of"
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
                                       << ValidatorDomainPrivate::dnsLookupTimeout;
#else
                                       << ValidatorDomainPrivate::dnsLookupTimeout.count()
                                       << "milliseconds";
#endif
    case ValidatorDomain::DNSError:
        qCDebug(C_VALIDATOR).noquote()
            << valInfo << "The DNS lookup failed because of errors in the"
            << "DNS resolution";
    }
}

ValidatorReturnType ValidatorDomain::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString &v = value(params);

    if (!v.isEmpty()) {
        Q_D(const ValidatorDomain);
        QString exVal;
        Diagnose diag{Valid};
        if (ValidatorDomain::validate(v, &diag, &exVal)) {
            result.value.setValue(exVal);
        } else {
            result.errorMessage = validationError(c, diag);
            if (C_VALIDATOR().isDebugEnabled()) {
                writeDebugString(debugString(c), diag, v);
            }
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

void ValidatorDomain::validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const
{
    const QString v = value(params);

    if (!v.isEmpty()) {
        Q_D(const ValidatorDomain);
        ValidatorDomain::validateCb(
            v, d->options, [cb, this, c, v](Diagnose diagnose, const QString &extractedValue) {
            if (diagnose == Valid) {
                cb({.errorMessage = {}, .value = extractedValue});
            } else {
                if (C_VALIDATOR().isDebugEnabled()) {
                    writeDebugString(debugString(c), diagnose, v);
                }
                cb({.errorMessage = validationError(c, diagnose)});
            }
        });
    } else {
        defaultValue(c, cb);
    }
}

QString ValidatorDomain::genericValidationError(Context *c, const QVariant &errorData) const
{
    return ValidatorDomain::diagnoseString(c, errorData.value<Diagnose>(), label(c));
}

#include "moc_validatordomain.cpp"
