/*
 * SPDX-FileCopyrightText: (C) 2018-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordomain_p.h"

#include <QDnsLookup>
#include <QEventLoop>
#include <QStringList>
#include <QTimer>
#include <QUrl>

using namespace Cutelyst;

ValidatorDomain::ValidatorDomain(const QString &field,
                                 bool checkDNS,
                                 const ValidatorMessages &messages,
                                 const QString &defValKey)
    : ValidatorRule(*new ValidatorDomainPrivate(field, checkDNS, messages, defValKey))
{
}

ValidatorDomain::~ValidatorDomain() = default;

bool ValidatorDomain::validate(const QString &value,
                               bool checkDNS,
                               Cutelyst::ValidatorDomain::Diagnose *diagnose,
                               QString *extractedValue)
{
    bool valid = true;

    Diagnose diag = Valid;

    QString _v      = value;
    bool hasRootDot = false;
    if (_v.endsWith(u'.')) {
        hasRootDot = true;
        _v.chop(1);
    }

    // convert to lower case puny code
    const QString v = QString::fromLatin1(QUrl::toAce(_v)).toLower();

    // split up the utf8 string into parts to get the non puny code TLD
    const QStringList nonAceParts = _v.split(QLatin1Char('.'));
    if (!nonAceParts.empty()) {
        const QString tld = nonAceParts.last();
        if (!tld.isEmpty()) {
            // there are no TLDs with digits inside, but IDN TLDs can
            // have digits in their puny code representation, so we have
            // to check at first if the IDN TLD contains digits before
            // checking the ACE puny code
            for (const QChar &ch : tld) {
                const ushort &uc = ch.unicode();
                if (((uc >= ValidatorRulePrivate::ascii_0) &&
                     (uc <= ValidatorRulePrivate::ascii_9)) ||
                    (uc == ValidatorRulePrivate::ascii_dash)) {
                    diag  = InvalidTLD;
                    valid = false;
                    break;
                }
            }

            if (valid) {
                if (!v.isEmpty()) {
                    // maximum length of the name in the DNS is 253 without the last dot
                    if (v.length() <= ValidatorDomainPrivate::maxDnsNameWithLastDot) {
                        const QStringList parts = v.split(QLatin1Char('.'), Qt::KeepEmptyParts);
                        // there has to be more than only the TLD
                        if (parts.size() > 1) {
                            // the TLD can not have only 1 char
                            if (parts.last().length() > 1) {
                                for (int i = 0; i < parts.size(); ++i) {
                                    if (valid) {
                                        const QString part = parts.at(i);
                                        if (!part.isEmpty()) {
                                            // labels/parts can have a maximum length of 63 chars
                                            if (part.length() <=
                                                ValidatorDomainPrivate::maxDnsLabelLength) {
                                                bool isTld      = (i == (parts.size() - 1));
                                                bool isPunyCode = part.startsWith(u"xn--");
                                                for (int j = 0; j < part.size(); ++j) {
                                                    const ushort &uc = part.at(j).unicode();
                                                    const bool isDigit =
                                                        ((uc >= ValidatorRulePrivate::ascii_0) &&
                                                         (uc <= ValidatorRulePrivate::ascii_9));
                                                    const bool isDash =
                                                        (uc == ValidatorRulePrivate::ascii_dash);
                                                    // no part/label can start with a digit or a
                                                    // dash
                                                    if ((j == 0) && (isDash || isDigit)) {
                                                        valid = false;
                                                        diag  = isDash ? DashStart : DigitStart;
                                                        break;
                                                    }
                                                    // no part/label can end with a dash
                                                    if ((j == (part.size() - 1)) && isDash) {
                                                        valid = false;
                                                        diag  = DashEnd;
                                                        break;
                                                    }
                                                    const bool isChar =
                                                        ((uc >= ValidatorRulePrivate::ascii_a) &&
                                                         (uc <= ValidatorRulePrivate::ascii_z));
                                                    if (!isTld) {
                                                        // if it is not the tld, it can have a-z 0-9
                                                        // and -
                                                        if (!(isDigit || isDash || isChar)) {
                                                            valid = false;
                                                            diag  = InvalidChars;
                                                            break;
                                                        }
                                                    } else {
                                                        if (isPunyCode) {
                                                            if (!(isDigit || isDash || isChar)) {
                                                                valid = false;
                                                                diag  = InvalidTLD;
                                                                break;
                                                            }
                                                        } else {
                                                            if (!isChar) {
                                                                valid = false;
                                                                diag  = InvalidTLD;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                            } else {
                                                valid = false;
                                                diag  = LabelTooLong;
                                                break;
                                            }
                                        } else {
                                            valid = false;
                                            diag  = EmptyLabel;
                                            break;
                                        }
                                    } else {
                                        break;
                                    }
                                }
                            } else {
                                valid = false;
                                diag  = InvalidTLD;
                            }
                        } else {
                            valid = false;
                            diag  = InvalidLabelCount;
                        }
                    } else {
                        valid = false;
                        diag  = TooLong;
                    }
                } else {
                    valid = false;
                    diag  = EmptyLabel;
                }
            }
        } else {
            valid = false;
            diag  = EmptyLabel;
        }
    } else {
        valid = false;
        diag  = EmptyLabel;
    }

    if (valid && checkDNS) {
        QDnsLookup alookup(QDnsLookup::A, v);
        QEventLoop aloop;
        QObject::connect(&alookup, &QDnsLookup::finished, &aloop, &QEventLoop::quit);
        QTimer::singleShot(ValidatorDomainPrivate::dnsLookupTimeout, &alookup, &QDnsLookup::abort);
        alookup.lookup();
        aloop.exec();

        if (((alookup.error() != QDnsLookup::NoError) &&
             (alookup.error() != QDnsLookup::OperationCancelledError)) ||
            alookup.hostAddressRecords().empty()) {
            QDnsLookup aaaaLookup(QDnsLookup::AAAA, v);
            QEventLoop aaaaLoop;
            QObject::connect(&aaaaLookup, &QDnsLookup::finished, &aaaaLoop, &QEventLoop::quit);
            QTimer::singleShot(
                ValidatorDomainPrivate::dnsLookupTimeout, &aaaaLookup, &QDnsLookup::abort);
            aaaaLookup.lookup();
            aaaaLoop.exec();

            if (((aaaaLookup.error() != QDnsLookup::NoError) &&
                 (aaaaLookup.error() != QDnsLookup::OperationCancelledError)) ||
                aaaaLookup.hostAddressRecords().empty()) {
                valid = false;
                diag  = MissingDNS;
            } else if (aaaaLookup.error() == QDnsLookup::OperationCancelledError) {
                valid = false;
                diag  = DNSTimeout;
            }
        } else if (alookup.error() == QDnsLookup::OperationCancelledError) {
            valid = false;
            diag  = DNSTimeout;
        }
    }

    if (diagnose) {
        *diagnose = diag;
    }

    if (valid && extractedValue) {
        if (hasRootDot) {
            *extractedValue = v + QLatin1Char('.');
        } else {
            *extractedValue = v;
        }
    }

    return valid;
}

QString ValidatorDomain::diagnoseString(Context *c, Diagnose diagnose, const QString &label)
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
        default:
            Q_ASSERT_X(false, "domain validation diagnose", "invalid diagnose");
            return {};
        }
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
        if (ValidatorDomain::validate(v, d->checkDNS, &diag, &exVal)) {
            result.value.setValue(exVal);
        } else {
            result.errorMessage = validationError(c, diag);
            if (C_VALIDATOR().isDebugEnabled()) {
                switch (diag) {
                case Valid:
                    break;
                case MissingDNS:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "Can not find valid DNS entry for" << v;
                    break;
                case InvalidChars:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c)
                        << "The domain name contains characters that are not allowed";
                    break;
                case LabelTooLong:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c)
                        << "At least on of the domain name labels exceeds the maximum"
                        << "size of" << ValidatorDomainPrivate::maxDnsLabelLength << "characters";
                    break;
                case TooLong:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "The domain name exceeds the maximum size of"
                        << ValidatorDomainPrivate::maxDnsNameWithLastDot << "characters";
                    break;
                case InvalidLabelCount:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "Invalid label count. Either no labels or only TLD";
                    break;
                case EmptyLabel:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "At least one of the domain name labels is empty";
                    break;
                case InvalidTLD:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c)
                        << "The TLD label contains characters that are not allowed";
                    break;
                case DashStart:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "At least one label starts with a dash";
                    break;
                case DashEnd:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "At least one label ends with a dash";
                    break;
                case DigitStart:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "At least one label starts with a digit";
                    break;
                case DNSTimeout:
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c) << "The DNS lookup exceeds the timeout of"
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
                        << ValidatorDomainPrivate::dnsLookupTimeout;
#else
                        << ValidatorDomainPrivate::dnsLookupTimeout.count() << "milliseconds";
#endif
                }
            }
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorDomain::genericValidationError(Context *c, const QVariant &errorData) const
{
    return ValidatorDomain::diagnoseString(c, errorData.value<Diagnose>(), label(c));
}

#include "moc_validatordomain.cpp"
