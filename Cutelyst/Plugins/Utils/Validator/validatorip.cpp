/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorip_p.h"

#include <utility>

#include <QHostAddress>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

const QRegularExpression ValidatorIpPrivate::regex{
    u"^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$"_s};

ValidatorIp::ValidatorIp(const QString &field,
                         Constraints constraints,
                         const Cutelyst::ValidatorMessages &messages,
                         const QString &defValKey)
    : ValidatorRule(*new ValidatorIpPrivate(field, constraints, messages, defValKey))
{
}

ValidatorIp::~ValidatorIp() = default;

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
            qCDebug(C_VALIDATOR).noquote() << "Not a valid IP address";
        }

    } else {
        defaultValue(c, &result);
    }

    return result;
}

void ValidatorIp::validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const
{
    cb(validate(c, params));
}

bool ValidatorIp::validate(const QString &value, Constraints constraints)
{
    bool valid = true;

    // simple check for an IPv4 address with four parts, because QHostAddress also tolerates
    // addresses like 192.168.2 and fills them with 0 somewhere
    if (!value.contains(QLatin1Char(':')) && !value.contains(ValidatorIpPrivate::regex)) {

        valid = false;

    } else {

        // private IPv4 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv4Private(
            {// Used for local communications within a private network
             // https://tools.ietf.org/html/rfc1918
             {QHostAddress(u"10.0.0.0"_s), 8},

             // Used for link-local addresses between two hosts on a single link when no IP address
             // is otherwise specified, such as would have normally been retrieved from a DHCP
             // server https://tools.ietf.org/html/rfc3927
             {QHostAddress(u"169.254.0.0"_s), 16},

             // Used for local communications within a private network
             // https://tools.ietf.org/html/rfc1918
             {QHostAddress(u"172.16.0.0"_s), 12},

             // Used for local communications within a private network
             // https://tools.ietf.org/html/rfc1918
             {QHostAddress(u"192.168.0.0"_s), 12}});

        // reserved IPv4 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv4Reserved(
            {// Used for broadcast messages to the current ("this")
             // https://tools.ietf.org/html/rfc1700
             {QHostAddress(u"0.0.0.0"_s), 8},

             // Used for communications between a service provider and its subscribers when using a
             // carrier-grade NAT https://tools.ietf.org/html/rfc6598
             {QHostAddress(u"100.64.0.0"_s), 10},

             // Used for loopback addresses to the local host
             // https://tools.ietf.org/html/rfc990
             {QHostAddress(u"127.0.0.1"_s), 8},

             // Used for the IANA IPv4 Special Purpose Address Registry
             // https://tools.ietf.org/html/rfc5736
             {QHostAddress(u"192.0.0.0"_s), 24},

             // Assigned as "TEST-NET" for use in documentation and examples. It should not be used
             // publicly. https://tools.ietf.org/html/rfc5737
             {QHostAddress(u"192.0.2.0"_s), 24},

             // Used by 6to4 anycast relays
             // https://tools.ietf.org/html/rfc3068
             {QHostAddress(u"192.88.99.0"_s), 24},

             // Used for testing of inter-network communications between two separate subnets
             // https://tools.ietf.org/html/rfc2544
             {QHostAddress(u"198.18.0.0"_s), 15},

             // Assigned as "TEST-NET-2" for use in documentation and examples. It should not be
             // used publicly. https://tools.ietf.org/html/rfc5737
             {QHostAddress(u"198.51.100.0"_s), 24},

             // Assigned as "TEST-NET-3" for use in documentation and examples. It should not be
             // used publicly. https://tools.ietf.org/html/rfc5737
             {QHostAddress(u"203.0.113.0"_s), 24},

             // Reserved for future use
             // https://tools.ietf.org/html/rfc6890
             {QHostAddress(u"240.0.0.0"_s), 4},

             // Reserved for the "limited broadcast" destination address
             // https://tools.ietf.org/html/rfc6890
             {QHostAddress(u"255.255.255.255"_s), 32}});

        // private IPv6 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv6Private(
            {// unique local address
             {QHostAddress(u"fc00::"_s), 7},

             // link-local address
             {QHostAddress(u"fe80::"_s), 10}});

        // reserved IPv6 subnets
        static const std::vector<std::pair<QHostAddress, int>> ipv6Reserved(
            {// unspecified address
             {QHostAddress(u"::"_s), 128},

             // loopback address to the local host
             {QHostAddress(u"::1"_s), 128},

             // IPv4 mapped addresses
             {QHostAddress(u"::ffff:0:0"_s), 96},

             // discard prefix
             // https://tools.ietf.org/html/rfc6666
             {QHostAddress(u"100::"_s), 64},

             // IPv4/IPv6 translation
             // https://tools.ietf.org/html/rfc6052
             {QHostAddress(u"64:ff9b::"_s), 96},

             // Teredo tunneling
             {QHostAddress(u"2001::"_s), 32},

             // deprected (previously ORCHID)
             {QHostAddress(u"2001:10::"_s), 28},

             // ORCHIDv2
             {QHostAddress(u"2001:20::"_s), 28},

             // addresses used in documentation and example source code
             {QHostAddress(u"2001:db8::"_s), 32},

             // 6to4
             {QHostAddress(u"2002::"_s), 16}});

        QHostAddress a;

        if (a.setAddress(value)) {

            if (!constraints.testFlag(NoConstraint)) {

                if (a.protocol() == QAbstractSocket::IPv4Protocol) {

                    if (constraints.testFlag(IPv6Only)) {
                        valid = false;
                    }

                    if (valid && constraints.testFlag(NoPrivateRange)) {
                        valid = !std::ranges::any_of(
                            ipv4Private, [&a](const std::pair<QHostAddress, int> &subnet) {
                            return a.isInSubnet(subnet.first, subnet.second);
                        });
                    }

                    if (valid && constraints.testFlag(NoReservedRange)) {
                        valid = !std::ranges::any_of(
                            ipv4Reserved, [&a](const std::pair<QHostAddress, int> &subnet) {
                            return a.isInSubnet(subnet.first, subnet.second);
                        });
                    }

                    if (valid && constraints.testFlag(NoMultiCast)) {
                        if (a.isInSubnet(QHostAddress(u"224.0.0.0"_s), 4)) {
                            valid = false;
                        }
                    }

                } else {

                    if (constraints.testFlag(IPv4Only)) {
                        valid = false;
                    }

                    if (valid && constraints.testFlag(NoPrivateRange)) {
                        valid = !std::ranges::any_of(
                            ipv6Private, [&a](const std::pair<QHostAddress, int> &subnet) {
                            return a.isInSubnet(subnet.first, subnet.second);
                        });
                    }

                    if (valid && constraints.testFlag(NoReservedRange)) {
                        valid = !std::ranges::any_of(
                            ipv6Reserved, [&a](const std::pair<QHostAddress, int> &subnet) {
                            return a.isInSubnet(subnet.first, subnet.second);
                        });
                    }

                    if (valid && constraints.testFlag(NoMultiCast)) {
                        if (a.isInSubnet(QHostAddress(u"ff00::"_s), 8)) {
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
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "IP address is invalid or not acceptable."
        return c->qtTrId("cutelyst-valip-genvalerr");
    } else {
        //: %1 will be replaced by the field label
        //% "The IP address in the “%1” field is invalid or not acceptable."
        return c->qtTrId("cutelyst-valip-genvalerr-label").arg(_label);
    }
}
