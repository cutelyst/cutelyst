/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "headers_p.h"

#include "common.h"

#include <QStringBuilder>
#include <QStringList>

#include <QDebug>

using namespace Cutelyst;

QString Headers::contentEncoding() const
{
    return value(QStringLiteral("Content-Encoding"));
}

void Headers::setContentEncoding(const QString &encoding)
{
    insert(QStringLiteral("Content-Encoding"), encoding);
}

QString Headers::contentType() const
{
    return value(QStringLiteral("Content-Type"));
}

void Headers::setContentType(const QString &contentType)
{
    insert(QStringLiteral("Content-Type"), contentType);
}

qint64 Headers::contentLength() const
{
    return value(QStringLiteral("Content-Length")).toLongLong();
}

void Headers::setContentLength(qint64 value)
{
    insert(QStringLiteral("Content-Length"), QString::number(value));
}

void Headers::setDateWithDateTime(const QDateTime &date)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    QLocale locale(QLocale::C);
    QString dt = locale.toString(date.toTimeSpec(Qt::UTC),
                                 QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
    insert(QStringLiteral("Date"), dt);
}

QString Headers::ifModifiedSince() const
{
    return value(QStringLiteral("If-Modified-Since"));
}

QDateTime Headers::ifModifiedSinceDateTime() const
{
    Headers::ConstIterator it = constFind(QStringLiteral("If-Modified-Since"));
    if (it == constEnd()) {
        return QDateTime();
    }

    const QString &ifModifiedStr = it.value();
    QLocale locale(QLocale::C);

    QDateTime localDT;
    if (ifModifiedStr.endsWith(QLatin1String(" GMT"))) {
        localDT = locale.toDateTime(ifModifiedStr.left(ifModifiedStr.size() - 4),
                                    QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
    } else {
        localDT = locale.toDateTime(ifModifiedStr,
                                    QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
    }
    return QDateTime(localDT.date(), localDT.time(), Qt::UTC);
}

QString Headers::lastModified() const
{
    return value(QStringLiteral("Last-Modified"));
}

void Headers::setLastModified(const QString &value)
{
    insert(QStringLiteral("Last-Modified"), value);
}

void Headers::setLastModified(const QDateTime &lastModified)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    QLocale locale(QLocale::C);
    QString dt = locale.toString(lastModified.toTimeSpec(Qt::UTC),
                                 QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
    setLastModified(dt);
}

QString Headers::server() const
{
    return value(QStringLiteral("Server"));
}

void Headers::setServer(const QString &value)
{
    insert(QStringLiteral("Server"), value);
}

QString Headers::userAgent() const
{
    return value(QStringLiteral("User-Agent"));
}

void Headers::setUserAgent(const QString &value)
{
    insert(QStringLiteral("User-Agent"), value);
}

QString Headers::referer() const
{
    return value(QStringLiteral("Referer"));
}

void Headers::setReferer(const QString &value)
{
    insert(QStringLiteral("Referer"), value);
}

void Headers::setWwwAuthenticate(const QString &value)
{
    insert(QStringLiteral("WWW-Authenticate"), value);
}

void Headers::setProxyAuthenticate(const QString &value)
{
    insert(QStringLiteral("Proxy-Authenticate"), value);
}

QString Headers::authorization() const
{
    return value(QStringLiteral("Authorization"));
}

QString Headers::authorizationBasic() const
{
    return HeadersPrivate::decodeBasicAuth(value(QStringLiteral("Authorization")));
}

QPair<QString, QString> Headers::authorizationBasicPair() const
{
    return HeadersPrivate::decodeBasicAuthPair(value(QStringLiteral("Authorization")));
}

void Headers::setAuthorizationBasic(const QString &username, const QString &password)
{
    if (username.contains(':')) {
        qCWarning(CUTELYST_CORE) << "Headers::Basic authorization user name can't contain ':'";
    }
    QString result = username % QLatin1Char(':') % password;
    insert(QStringLiteral("Authorization"), QStringLiteral("Basic ") + result.toLatin1().toBase64());
}

QString Headers::proxyAuthorization() const
{
    return value(QStringLiteral("Proxy-Authorization"));
}

QString Headers::proxyAuthorizationBasic() const
{
    return HeadersPrivate::decodeBasicAuth(value(QStringLiteral("Proxy-Authorization")));
}

QPair<QString, QString> Headers::proxyAuthorizationBasicPair() const
{
    return HeadersPrivate::decodeBasicAuthPair(value(QStringLiteral("Proxy-Authorization")));
}

QString Headers::header(const QString &field) const
{
    return value(field);
}

void Headers::setHeader(const QString &field, const QStringList &values)
{
    insert(field, values.join(QLatin1String(", ")));
}

static QString cutelyst_header_order(
        // General headers
        "Cache-Control\n"
        "Connection\n"
        "Date\n"
        "Pragma\n"
        "Trailer\n"
        "Transfer-Encoding\n"
        "Upgrade\n"
        "Via\n"
        "Warning\n"
        // Request headers
        "Accept\n"
        "Accept-Charset\n"
        "Accept-Encoding\n"
        "Accept-Language\n"
        "Authorization\n"
        "Expect\n"
        "From\n"
        "Host\n"
        "If-Match\n"
        "If-Modified-Since\n"
        "If-None-Match\n"
        "If-Range\n"
        "If-Unmodified-Since\n"
        "Max-Forwards\n"
        "Proxy-Authorization\n"
        "Range\n"
        "Referer\n"
        "TE\n"
        "User-Agent\n"
        // Response headers
        "Accept-Ranges\n"
        "Age\n"
        "ETag\n"
        "Location\n"
        "Proxy-Authenticate\n"
        "Retry-After\n"
        "Server\n"
        "Vary\n"
        "WWW-Authenticate\n"
        // Entity headers
        "Allow\n"
        "Content-Encoding\n"
        "Content-Language\n"
        "Content-Length\n"
        "Content-Location\n"
        "Content-MD5\n"
        "Content-Range\n"
        "Content-Type\n"
        "Expires\n"
        "Last-Modified"
        );

bool httpGoodPracticeWeightSort(const HeaderValuePair &pair1, const HeaderValuePair &pair2)
{
    int index1 = pair1.weight;
    int index2 = pair2.weight;

    if (index1 != -1 && index2 != -1) {
        // Both items are in the headerOrder list
        return index1 < index2;
    } else if (index1 == -1 && index2 == -1) {
        // Noone of them are int the headerOrder list
        return false;
    }

    // if the pair1 is in the header list it should go first
    return index1 != -1;
}

QList<HeaderValuePair> Headers::headersForResponse() const
{
    QList<HeaderValuePair> ret;

    QHash<QString, QString>::const_iterator it = constBegin();
    while (it != constEnd()) {
        HeaderValuePair pair;
        pair.key = it.key();
        pair.value = it.value();
        pair.weight = cutelyst_header_order.indexOf(pair.key);
        ret.append(pair);
        ++it;
    }

    // Sort base on the "good practices" of HTTP RCF
    qSort(ret.begin(), ret.end(), &httpGoodPracticeWeightSort);

    return ret;
}


QByteArray HeadersPrivate::decodeBasicAuth(const QString &auth)
{
    if (!auth.isEmpty() && auth.startsWith("Basic ")) {
        int pos = auth.lastIndexOf(' ');
        if (pos != -1) {
            return QByteArray::fromBase64(auth.mid(pos).toLatin1());
        }
    }
    return QByteArray();
}

QPair<QString, QString> HeadersPrivate::decodeBasicAuthPair(const QString &auth)
{
    QPair<QString, QString> ret;
    const QByteArray &authorization = decodeBasicAuth(auth);
    if (!authorization.isEmpty()) {
        int pos = authorization.indexOf(':');
        if (pos == -1) {
            ret.first = QString::fromLatin1(authorization);
        } else {
            ret.first = QString::fromLatin1(authorization.left(pos));
            ret.second = QString::fromLatin1(authorization.mid(pos + 1));
        }
    }
    return ret;
}
