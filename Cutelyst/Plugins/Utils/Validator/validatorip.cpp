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

#include "validatorip_p.h"
#include <QHostAddress>
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorIp::ValidatorIp(const QString &field, Constraints constraints, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorIpPrivate(field, constraints, label, customError), parent)
{

}


ValidatorIp::ValidatorIp(ValidatorIpPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorIp::~ValidatorIp()
{

}



bool ValidatorIp::validate()
{
    Q_D(ValidatorIp);

    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    // simple check for an IPv4 address with four parts, because QHostAddress also tolerates addresses like 192.168.2 and fills them with 0 somewhere
    if (!v.contains(QStringLiteral(":")) && !v.contains(QRegularExpression(QStringLiteral("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$")))) {
        return false;
    }

    QHostAddress a;
    if (a.setAddress(value())) {

        if (d->constraints.testFlag(NoConstraint)) {

            setValid(true);
            return true;

        } else {

            if (a.protocol() == QAbstractSocket::IPv4Protocol) {

                if (d->constraints.testFlag(IPv6Only)) {
                    return false;
                }

                if (d->constraints.testFlag(NoPrivateRange) || d->constraints.testFlag(PublicOnly)) {
                    // Used for local communications within a private network
                    // https://tools.ietf.org/html/rfc1918
                    if (a.isInSubnet(QHostAddress(QStringLiteral("10.0.0.0")), 8)) {
                        return false;
                    }

                    // Used for link-local addresses between two hosts on a single link when no IP address is otherwise specified, such as would have normally been retrieved from a DHCP server
                    // https://tools.ietf.org/html/rfc3927
                    if (a.isInSubnet(QHostAddress(QStringLiteral("169.254.0.0")), 16)) {
                        return false;
                    }

                    // Used for local communications within a private network
                    // https://tools.ietf.org/html/rfc1918
                    if (a.isInSubnet(QHostAddress(QStringLiteral("172.16.0.0")), 12)) {
                        return false;
                    }

                    // Used for local communications within a private network
                    // https://tools.ietf.org/html/rfc1918
                    if (a.isInSubnet(QHostAddress(QStringLiteral("192.168.0.0")), 12)) {
                        return false;
                    }
                }

                if (d->constraints.testFlag(NoReservedRange) || d->constraints.testFlag(PublicOnly)) {
                    // Used for broadcast messages to the current ("this")
                    // https://tools.ietf.org/html/rfc1700
                    if (a.isInSubnet(QHostAddress(QStringLiteral("0.0.0.0")), 8)) {
                        return false;
                    }

                    // Used for communications between a service provider and its subscribers when using a carrier-grade NAT
                    // https://tools.ietf.org/html/rfc6598
                    if (a.isInSubnet(QHostAddress(QStringLiteral("100.64.0.0")), 10)) {
                        return false;
                    }

                    // Used for loopback addresses to the local host
                    // https://tools.ietf.org/html/rfc990
                    if (a.isInSubnet(QHostAddress(QStringLiteral("127.0.0.1")), 8)) {
                        return false;
                    }

                    // Used for the IANA IPv4 Special Purpose Address Registry
                    // https://tools.ietf.org/html/rfc5736
                    if (a.isInSubnet(QHostAddress(QStringLiteral("192.0.0.0")), 24)) {
                        return false;
                    }

                    // Assigned as "TEST-NET" for use in documentation and examples. It should not be used publicly.
                    // https://tools.ietf.org/html/rfc5737
                    if (a.isInSubnet(QHostAddress(QStringLiteral("192.0.2.0")), 24)) {
                        return false;
                    }

                    // Used by 6to4 anycast relays
                    // https://tools.ietf.org/html/rfc3068
                    if (a.isInSubnet(QHostAddress(QStringLiteral("192.88.99.0")), 24)) {
                        return false;
                    }

                    // Used for testing of inter-network communications between two separate subnets
                    // https://tools.ietf.org/html/rfc2544
                    if (a.isInSubnet(QHostAddress(QStringLiteral("198.18.0.0")), 15)) {
                        return false;
                    }

                    // Assigned as "TEST-NET-2" for use in documentation and examples. It should not be used publicly.
                    // https://tools.ietf.org/html/rfc5737
                    if (a.isInSubnet(QHostAddress(QStringLiteral("198.51.100.0")), 24)) {
                        return false;
                    }

                    // Assigned as "TEST-NET-3" for use in documentation and examples. It should not be used publicly.
                    // https://tools.ietf.org/html/rfc5737
                    if (a.isInSubnet(QHostAddress(QStringLiteral("203.0.113.0")), 24)) {
                        return false;
                    }

                    // Reserved for future use
                    // https://tools.ietf.org/html/rfc6890
                    if (a.isInSubnet(QHostAddress(QStringLiteral("240.0.0.0")), 4)) {
                        return false;
                    }

                    // Reserved for the "limited broadcast" destination address
                    // https://tools.ietf.org/html/rfc6890
                    if (a.isInSubnet(QHostAddress(QStringLiteral("255.255.255.255")), 32)) {
                        return false;
                    }
                }

                if (d->constraints.testFlag(NoMultiCast) || d->constraints.testFlag(PublicOnly)) {
                    if (a.isInSubnet(QHostAddress(QStringLiteral("224.0.0.0")), 4)) {
                        return false;
                    }
                }

                setValid(true);
                return true;

            } else {

                if (d->constraints.testFlag(IPv4Only)) {
                    return false;
                }

                if (d->constraints.testFlag(NoPrivateRange) || d->constraints.testFlag(PublicOnly)) {

                    // unique local address
                    if (a.isInSubnet(QHostAddress(QStringLiteral("fc00::")), 7)) {
                        return false;
                    }

                    // link-local address
                    if (a.isInSubnet(QHostAddress(QStringLiteral("fe80::")), 10)) {
                        return false;
                    }
                }

                if (d->constraints.testFlag(NoReservedRange) || d->constraints.testFlag(PublicOnly)) {

                    // unspecified address
                    if (a.isInSubnet(QHostAddress(QStringLiteral("::")), 128)) {
                        return false;
                    }

                    // loopback address to the loca host
                    if (a.isInSubnet(QHostAddress(QStringLiteral("::1")), 128)) {
                        return false;
                    }

                    // IPv4 mapped addresses
                    if (a.isInSubnet(QHostAddress(QStringLiteral("::ffff:0:0")), 96)) {
                        return false;
                    }

                    // discard prefix
                    // https://tools.ietf.org/html/rfc6666
                    if (a.isInSubnet(QHostAddress(QStringLiteral("100::")), 64)) {
                        return false;
                    }

                    // IPv4/IPv6 translation
                    // https://tools.ietf.org/html/rfc6052
                    if (a.isInSubnet(QHostAddress(QStringLiteral("64:ff9b::")), 96)) {
                        return false;
                    }

                    // Teredo tunneling
                    if (a.isInSubnet(QHostAddress(QStringLiteral("2001::")), 32)) {
                        return false;
                    }

                    // deprected (previously ORCHID)
                    if (a.isInSubnet(QHostAddress(QStringLiteral("2001:10::")), 28)) {
                        return false;
                    }

                    // ORCHIDv2
                    if (a.isInSubnet(QHostAddress(QStringLiteral("2001:20::")), 28)) {
                        return false;
                    }

                    // addresses used in documentation and example source code
                    if (a.isInSubnet(QHostAddress(QStringLiteral("2001:db8::")), 32)) {
                        return false;
                    }

                    // 6to4
                    if (a.isInSubnet(QHostAddress(QStringLiteral("2002::")), 16)) {
                        return false;
                    }
                }

                if (d->constraints.testFlag(NoMultiCast) || d->constraints.testFlag(PublicOnly)) {
                    if (a.isInSubnet(QHostAddress(QStringLiteral("ff00::")), 8)) {
                        return false;
                    }
                }

                setValid(true);
                return true;

            }

        }
    }

    return false;
}



QString ValidatorIp::genericErrorMessage() const
{
    return tr("You have to enter a valid IP address into the “%1“ field.").arg(genericFieldName());
}



void ValidatorIp::setConstraints(Constraints constraints)
{
    Q_D(ValidatorIp);
    d->constraints = constraints;
}
