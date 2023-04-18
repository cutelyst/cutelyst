/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorip_p.h"

#include <utility>

#include <QHostAddress>
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorIp::ValidatorIp(const QString &field, Constraints constraints, const Cutelyst::ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorIpPrivate(field, constraints, messages, defValKey))
{
}

ValidatorIp::~ValidatorIp()
{
}

ValidatorReturnType ValidatorIp::validate(Cutelyst::Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorIp);

    const QString v = value(params);

    if (!v.isEmpty()) {

        if (ValidatorIp::validate(v, d->constraints)) {
            result.value.setValue(v);
        } else {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR, "ValidatorIp: Validation failed for field %s at %s::%s: not a valid IP address within the constraints.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        }

    } else {
        defaultValue(c, &result, "ValidatorIp");
    }

    return result;
}

bool ValidatorIp::validate(const QString &value, Constraints constraints)
{
    bool valid = true;

    // simple check for an IPv4 address with four parts, because QHostAddress also tolerates addresses like 192.168.2 and fills them with 0 somewhere
    if (!value.contains(QLatin1Char(':')) && !value.contains(QRegularExpression(QStringLiteral("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$")))) {

        valid = false;

    } else {

        // private IPv4 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv4Private({// Used for local communications within a private network
                                                                            // https://tools.ietf.org/html/rfc1918
                                                                            {QHostAddress(QStringLiteral("10.0.0.0")), 8},

                                                                            // Used for link-local addresses between two hosts on a single link when no IP address
                                                                            // is otherwise specified, such as would have normally been retrieved from a DHCP server
                                                                            // https://tools.ietf.org/html/rfc3927
                                                                            {QHostAddress(QStringLiteral("169.254.0.0")), 16},

                                                                            // Used for local communications within a private network
                                                                            // https://tools.ietf.org/html/rfc1918
                                                                            {QHostAddress(QStringLiteral("172.16.0.0")), 12},

                                                                            // Used for local communications within a private network
                                                                            // https://tools.ietf.org/html/rfc1918
                                                                            {QHostAddress(QStringLiteral("192.168.0.0")), 12}});

        // reserved IPv4 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv4Reserved({// Used for broadcast messages to the current ("this")
                                                                             // https://tools.ietf.org/html/rfc1700
                                                                             {QHostAddress(QStringLiteral("0.0.0.0")), 8},

                                                                             // Used for communications between a service provider and its subscribers when using a carrier-grade NAT
                                                                             // https://tools.ietf.org/html/rfc6598
                                                                             {QHostAddress(QStringLiteral("100.64.0.0")), 10},

                                                                             // Used for loopback addresses to the local host
                                                                             // https://tools.ietf.org/html/rfc990
                                                                             {QHostAddress(QStringLiteral("127.0.0.1")), 8},

                                                                             // Used for the IANA IPv4 Special Purpose Address Registry
                                                                             // https://tools.ietf.org/html/rfc5736
                                                                             {QHostAddress(QStringLiteral("192.0.0.0")), 24},

                                                                             // Assigned as "TEST-NET" for use in documentation and examples. It should not be used publicly.
                                                                             // https://tools.ietf.org/html/rfc5737
                                                                             {QHostAddress(QStringLiteral("192.0.2.0")), 24},

                                                                             // Used by 6to4 anycast relays
                                                                             // https://tools.ietf.org/html/rfc3068
                                                                             {QHostAddress(QStringLiteral("192.88.99.0")), 24},

                                                                             // Used for testing of inter-network communications between two separate subnets
                                                                             // https://tools.ietf.org/html/rfc2544
                                                                             {QHostAddress(QStringLiteral("198.18.0.0")), 15},

                                                                             // Assigned as "TEST-NET-2" for use in documentation and examples. It should not be used publicly.
                                                                             // https://tools.ietf.org/html/rfc5737
                                                                             {QHostAddress(QStringLiteral("198.51.100.0")), 24},

                                                                             // Assigned as "TEST-NET-3" for use in documentation and examples. It should not be used publicly.
                                                                             // https://tools.ietf.org/html/rfc5737
                                                                             {QHostAddress(QStringLiteral("203.0.113.0")), 24},

                                                                             // Reserved for future use
                                                                             // https://tools.ietf.org/html/rfc6890
                                                                             {QHostAddress(QStringLiteral("240.0.0.0")), 4},

                                                                             // Reserved for the "limited broadcast" destination address
                                                                             // https://tools.ietf.org/html/rfc6890
                                                                             {QHostAddress(QStringLiteral("255.255.255.255")), 32}});

        // private IPv6 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv6Private({// unique local address
                                                                            {QHostAddress(QStringLiteral("fc00::")), 7},

                                                                            // link-local address
                                                                            {QHostAddress(QStringLiteral("fe80::")), 10}});

        // reserved IPv6 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv6Reserved({// unspecified address
                                                                             {QHostAddress(QStringLiteral("::")), 128},

                                                                             // loopback address to the loca host
                                                                             {QHostAddress(QStringLiteral("::1")), 128},

                                                                             // IPv4 mapped addresses
                                                                             {QHostAddress(QStringLiteral("::ffff:0:0")), 96},

                                                                             // discard prefix
                                                                             // https://tools.ietf.org/html/rfc6666
                                                                             {QHostAddress(QStringLiteral("100::")), 64},

                                                                             // IPv4/IPv6 translation
                                                                             // https://tools.ietf.org/html/rfc6052
                                                                             {QHostAddress(QStringLiteral("64:ff9b::")), 96},

                                                                             // Teredo tunneling
                                                                             {QHostAddress(QStringLiteral("2001::")), 32},

                                                                             // deprected (previously ORCHID)
                                                                             {QHostAddress(QStringLiteral("2001:10::")), 28},

                                                                             // ORCHIDv2
                                                                             {QHostAddress(QStringLiteral("2001:20::")), 28},

                                                                             // addresses used in documentation and example source code
                                                                             {QHostAddress(QStringLiteral("2001:db8::")), 32},

                                                                             // 6to4
                                                                             {QHostAddress(QStringLiteral("2002::")), 16}});

        QHostAddress a;

        if (a.setAddress(value)) {

            if (!constraints.testFlag(NoConstraint)) {

                if (a.protocol() == QAbstractSocket::IPv4Protocol) {

                    if (constraints.testFlag(IPv6Only)) {
                        valid = false;
                    }

                    if (valid && (constraints.testFlag(NoPrivateRange) || constraints.testFlag(PublicOnly))) {

                        for (const std::pair<QHostAddress, int> &subnet : ipv4Private) {
                            if (a.isInSubnet(subnet.first, subnet.second)) {
                                valid = false;
                                break;
                            }
                        }
                    }

                    if (valid && (constraints.testFlag(NoReservedRange) || constraints.testFlag(PublicOnly))) {

                        for (const std::pair<QHostAddress, int> &subnet : ipv4Reserved) {
                            if (a.isInSubnet(subnet.first, subnet.second)) {
                                valid = false;
                                break;
                            }
                        }
                    }

                    if (valid && (constraints.testFlag(NoMultiCast) || constraints.testFlag(PublicOnly))) {
                        if (a.isInSubnet(QHostAddress(QStringLiteral("224.0.0.0")), 4)) {
                            valid = false;
                        }
                    }

                } else {

                    if (constraints.testFlag(IPv4Only)) {
                        valid = false;
                    }

                    if (valid && (constraints.testFlag(NoPrivateRange) || constraints.testFlag(PublicOnly))) {

                        for (const std::pair<QHostAddress, int> &subnet : ipv6Private) {
                            if (a.isInSubnet(subnet.first, subnet.second)) {
                                valid = false;
                                break;
                            }
                        }
                    }

                    if (valid && (constraints.testFlag(NoReservedRange) || constraints.testFlag(PublicOnly))) {

                        for (const std::pair<QHostAddress, int> &subnet : ipv6Reserved) {
                            if (a.isInSubnet(subnet.first, subnet.second)) {
                                valid = false;
                                break;
                            }
                        }
                    }

                    if (valid && (constraints.testFlag(NoMultiCast) || constraints.testFlag(PublicOnly))) {
                        if (a.isInSubnet(QHostAddress(QStringLiteral("ff00::")), 8)) {
                            valid = false;
                        }
                    }
                }
            }

        } else {
            valid = false;
        }
    }

    return valid;
}

QString ValidatorIp::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorIp", "IP address is invalid or not acceptable.");
    } else {
        //: %1 will be replaced by the field label
        error = c->translate("Cutelyst::ValidatorIp", "The IP address in the “%1” field is invalid or not acceptable.").arg(_label);
    }
    return error;
}
