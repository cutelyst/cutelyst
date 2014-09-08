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

void Headers::setDateWithDateTime(const QDateTime &date)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    QString dt = date.toTimeSpec(Qt::UTC).toString(QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
    insert("Date", dt.toLocal8Bit());
}

void Headers::setLastModifiedDateTime(const QDateTime &lastModified)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    QString dt = lastModified.toTimeSpec(Qt::UTC).toString(QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
    insert("Last-Modified", dt.toLocal8Bit());
}

QByteArray Headers::authorizationBasic() const
{
    return HeadersPrivate::decodeBasicAuth(value(QByteArrayLiteral("Authorization")));
}

QPair<QByteArray, QByteArray> Headers::authorizationBasicPair() const
{
    return HeadersPrivate::decodeBasicAuthPair(value(QByteArrayLiteral("Authorization")));
}

void Headers::setAuthorizationBasic(const QByteArray &username, const QByteArray &password)
{
    if (username.contains(':')) {
        qCWarning(CUTELYST_CORE) << "Headers::Basic authorization user name can't contain ':'";
    }
    QByteArray result = username + ':' + password;
    insert(QByteArrayLiteral("Authorization"), QByteArrayLiteral("Basic ") + result.toBase64());
}

QByteArray Headers::proxyAuthorizationBasic() const
{
    return HeadersPrivate::decodeBasicAuth(value(QByteArrayLiteral("Proxy-Authorization")));
}

QPair<QByteArray, QByteArray> Headers::proxyAuthorizationBasicPair() const
{
    return HeadersPrivate::decodeBasicAuthPair(value(QByteArrayLiteral("Proxy-Authorization")));
}

void Headers::setHeader(const QString &field, const QStringList &values)
{
    insert(field.toLocal8Bit(), values.join(QLatin1String(", ")).toLocal8Bit());
}

static QByteArray cutelyst_header_order(
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

    QHash<QByteArray, QByteArray>::const_iterator it = constBegin();
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


QByteArray HeadersPrivate::decodeBasicAuth(const QByteArray &auth)
{
    if (!auth.isEmpty() && auth.startsWith("Basic ")) {
        int pos = auth.lastIndexOf(' ');
        if (pos != -1) {
            return QByteArray::fromBase64(auth.mid(pos));
        }
    }
    return QByteArray();
}

QPair<QByteArray, QByteArray> HeadersPrivate::decodeBasicAuthPair(const QByteArray &auth)
{
    QPair<QByteArray, QByteArray> ret;
    const QByteArray &authorization = decodeBasicAuth(auth);
    if (!authorization.isEmpty()) {
        int pos = authorization.indexOf(':');
        if (pos == -1) {
            ret.first = authorization;
        } else {
            ret.first = authorization.left(pos);
            ret.second = authorization.mid(pos + 1);
        }
    }
    return ret;
}
