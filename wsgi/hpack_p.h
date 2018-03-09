/*
 * Copyright (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef HPACK_P_H
#define HPACK_P_H

#include <QString>







static const std::pair<QString, QString> hpackStaticHeaders[] = {
    {QString(), QString()},
    {QStringLiteral(":authority"), QString()},
    {QStringLiteral(":method"), QStringLiteral("GET")},
    {QStringLiteral(":method"), QStringLiteral("POST")},
    {QStringLiteral(":path"), QStringLiteral("/")},
    {QStringLiteral(":path"), QStringLiteral("/index.html")},
    {QStringLiteral(":scheme"), QStringLiteral("http")},
    {QStringLiteral(":scheme"), QStringLiteral("https")},
    {QStringLiteral(":status"), QStringLiteral("200")},
    {QStringLiteral(":status"), QStringLiteral("204")},
    {QStringLiteral(":status"), QStringLiteral("206")},
    {QStringLiteral(":status"), QStringLiteral("304")},
    {QStringLiteral(":status"), QStringLiteral("400")},
    {QStringLiteral(":status"), QStringLiteral("404")},
    {QStringLiteral(":status"), QStringLiteral("500")},
    {QStringLiteral("accept-charset"), QString()},
    {QStringLiteral("accept-encoding"), QStringLiteral("gzip, deflate")},
    {QStringLiteral("accept-language"), QString()},
    {QStringLiteral("accept-ranges"), QString()},
    {QStringLiteral("accept"), QString()},
    {QStringLiteral("access-control-allow-origin"), QString()},
    {QStringLiteral("age"), QString()},
    {QStringLiteral("allow"), QString()},
    {QStringLiteral("authorization"), QString()},
    {QStringLiteral("cache-control"), QString()},
    {QStringLiteral("content-disposition"), QString()},
    {QStringLiteral("content-encoding"), QString()},
    {QStringLiteral("content-language"), QString()},
    {QStringLiteral("content-length"), QString()},
    {QStringLiteral("content-location"), QString()},
    {QStringLiteral("content-range"), QString()},
    {QStringLiteral("content-type"), QString()},
    {QStringLiteral("cookie"), QString()},
    {QStringLiteral("date"), QString()},
    {QStringLiteral("etag"), QString()},
    {QStringLiteral("expect"), QString()},
    {QStringLiteral("expires"), QString()},
    {QStringLiteral("from"), QString()},
    {QStringLiteral("host"), QString()},
    {QStringLiteral("if-match"), QString()},
    {QStringLiteral("if-modified-since"), QString()},
    {QStringLiteral("if-none-match"), QString()},
    {QStringLiteral("if-range"), QString()},
    {QStringLiteral("if-unmodified-since"), QString()},
    {QStringLiteral("last-modified"), QString()},
    {QStringLiteral("link"), QString()},
    {QStringLiteral("location"), QString()},
    {QStringLiteral("max-forwards"), QString()},
    {QStringLiteral("proxy-authenticate"), QString()},
    {QStringLiteral("proxy-authorization"), QString()},
    {QStringLiteral("range"), QString()},
    {QStringLiteral("referer"), QString()},
    {QStringLiteral("refresh"), QString()},
    {QStringLiteral("retry-after"), QString()},
    {QStringLiteral("server"), QString()},
    {QStringLiteral("set-cookie"), QString()},
    {QStringLiteral("strict-transport-security"), QString()},
    {QStringLiteral("transfer-encoding"), QString()},
    {QStringLiteral("user-agent"), QString()},
    {QStringLiteral("vary"), QString()},
    {QStringLiteral("via"), QString()},
    {QStringLiteral("www-authenticate"), QString()}
};

#endif // HPACK_P_H
