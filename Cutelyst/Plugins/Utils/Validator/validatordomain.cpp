/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordomain_p.h"

#include <QDnsLookup>
#include <QEventLoop>
#include <QStringList>
#include <QTimer>
#include <QUrl>

using namespace Cutelyst;

ValidatorDomain::ValidatorDomain(const QString &field, bool checkDNS, const ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorDomainPrivate(field, checkDNS, messages, defValKey))
{
}

ValidatorDomain::~ValidatorDomain()
{
}

bool ValidatorDomain::validate(const QString &value, bool checkDNS, Cutelyst::ValidatorDomain::Diagnose *diagnose, QString *extractedValue)
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
                if (((uc > 47) && (uc < 58)) || (uc == 45)) {
                    diag  = InvalidTLD;
                    valid = false;
                    break;
                }
            }

            if (valid) {
                if (!v.isEmpty()) {
                    // maximum length of the name in the DNS is 253 without the last dot
                    if (v.length() < 254) {
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
                                            if (part.length() < 64) {
                                                bool isTld      = (i == (parts.size() - 1));
                                                bool isPunyCode = part.startsWith(u"xn--");
                                                for (int j = 0; j < part.size(); ++j) {
                                                    const ushort &uc   = part.at(j).unicode();
                                                    const bool isDigit = ((uc > 47) && (uc < 58));
                                                    const bool isDash  = (uc == 45);
                                                    // no part/label can start with a digit or a dash
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
                                                    const bool isChar = ((uc > 96) && (uc < 123));
                                                    if (!isTld) {
                                                        // if it is not the tld, it can have a-z 0-9 and -
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
        QTimer::singleShot(3100, &alookup, &QDnsLookup::abort);
        alookup.lookup();
        aloop.exec();

        if (((alookup.error() != QDnsLookup::NoError) && (alookup.error() != QDnsLookup::OperationCancelledError)) || alookup.hostAddressRecords().empty()) {
            QDnsLookup aaaaLookup(QDnsLookup::AAAA, v);
            QEventLoop aaaaLoop;
            QObject::connect(&aaaaLookup, &QDnsLookup::finished, &aaaaLoop, &QEventLoop::quit);
            QTimer::singleShot(3100, &aaaaLookup, &QDnsLookup::abort);
            aaaaLookup.lookup();
            aaaaLoop.exec();

            if (((aaaaLookup.error() != QDnsLookup::NoError) && (aaaaLookup.error() != QDnsLookup::OperationCancelledError)) || aaaaLookup.hostAddressRecords().empty()) {
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
    QString error;

    if (label.isEmpty()) {
        switch (diagnose) {
        case MissingDNS:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name seems to be valid but could not be found in the domain name system.");
            break;
        case InvalidChars:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name contains characters that are not allowed.");
            break;
        case LabelTooLong:
            error = c->translate("Cutelyst::ValidatorDomain", "At least one of the sections separated by dots exceeds the maximum allowed length of 63 characters. Note that internationalized domain names can be longer internally than they are displayed.");
            break;
        case TooLong:
            error = c->translate("Cutelyst::ValidatorDomain", "The full name of the domain must not be longer than 253 characters. Note that internationalized domain names can be longer internally than they are displayed.");
            break;
        case InvalidLabelCount:
            error = c->translate("Cutelyst::ValidatorDomain", "This is not a valid domain name because it has either no parts (is empty) or only has a top level domain.");
            break;
        case EmptyLabel:
            error = c->translate("Cutelyst::ValidatorDomain", "At least one of the sections separated by dots is empty. Check whether you have entered two dots consecutively.");
            break;
        case InvalidTLD:
            error = c->translate("Cutelyst::ValidatorDomain", "The top level domain (last part) contains characters that are not allowed, like digits and/or dashes.");
            break;
        case DashStart:
            error = c->translate("Cutelyst::ValidatorDomain", "Domain name sections are not allowed to start with a dash.");
            break;
        case DashEnd:
            error = c->translate("Cutelyst::ValidatorDomain", "Domain name sections are not allowed to end with a dash.");
            break;
        case DigitStart:
            error = c->translate("Cutelyst::ValidatorDomain", "Domain name sections are not allowed to start with a digit.");
            break;
        case Valid:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name is valid.");
            break;
        case DNSTimeout:
            error = c->translate("Cutelyst::ValidatorDomain", "The DNS lookup was aborted because it took too long.");
            break;
        default:
            Q_ASSERT_X(false, "domain validation diagnose", "invalid diagnose");
            break;
        }
    } else {
        switch (diagnose) {
        case MissingDNS:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1“ field seems to be valid but could not be found in the domain name system.").arg(label);
            break;
        case InvalidChars:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1“ field contains characters that are not allowed.").arg(label);
            break;
        case LabelTooLong:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1“ field is not valid because at least one of the sections separated by dots exceeds the maximum allowed length of 63 characters. Note that internationalized domain names can be longer internally than they are displayed.").arg(label);
            break;
        case TooLong:
            error = c->translate("Cutelyst::ValidatorDomain", "The full name of the domain in the “%1” field must not be longer than 253 characters. Note that internationalized domain names can be longer internally than they are displayed.").arg(label);
            break;
        case InvalidLabelCount:
            error = c->translate("Cutelyst::ValidatorDomain", "The “%1” field does not contain a valid domain name because it has either no parts (is empty) or only has a top level domain.").arg(label);
            break;
        case EmptyLabel:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1“ field is not valid because at least one of the sections separated by dots is empty. Check whether you have entered two dots consecutively.").arg(label);
            break;
        case InvalidTLD:
            error = c->translate("Cutelyst::ValidatorDomain", "The top level domain (last part) of the domain name in the “%1” field contains characters that are not allowed, like digits and or dashes.").arg(label);
            break;
        case DashStart:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1“ field is not valid because domain name sections are not allowed to start with a dash.").arg(label);
            break;
        case DashEnd:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1“ field is not valid because domain name sections are not allowed to end with a dash.").arg(label);
            break;
        case DigitStart:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1“ field is not valid because domain name sections are not allowed to start with a digit.").arg(label);
            break;
        case Valid:
            error = c->translate("Cutelyst::ValidatorDomain", "The domain name in the “%1” field is valid.").arg(label);
            break;
        case DNSTimeout:
            error = c->translate("Cutelyst::ValidatorDomain", "The DNS lookup for the domain name in the “%1” field was aborted because it took too long.").arg(label);
            break;
        default:
            Q_ASSERT_X(false, "domain validation diagnose", "invalid diagnose");
            break;
        }
    }

    return error;
}

ValidatorReturnType ValidatorDomain::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString &v = value(params);

    if (!v.isEmpty()) {
        Q_D(const ValidatorDomain);
        QString exVal;
        Diagnose diag;
        if (ValidatorDomain::validate(v, d->checkDNS, &diag, &exVal)) {
            result.value.setValue(exVal);
        } else {
            result.errorMessage = validationError(c, diag);
        }
    } else {
        defaultValue(c, &result, "ValidatorDomain");
    }

    return result;
}

QString ValidatorDomain::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    const QString _label = label(c);
    const Diagnose diag  = errorData.value<Diagnose>();
    error                = ValidatorDomain::diagnoseString(c, diag, _label);
    return error;
}

#include "moc_validatordomain.cpp"
