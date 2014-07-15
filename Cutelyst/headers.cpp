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

#include <QStringBuilder>
#include <QStringList>

#include <QDebug>

using namespace Cutelyst;

// TODO see if fromString is enough and make it inline
QDateTime Headers::date() const
{
    return QDateTime::fromString(value("Date"));
}

void Headers::setDate(const QDateTime &date)
{
    QString dateString;
    // TODO check if there is some RFC format
    dateString = date.toString(QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
    insert("Date", dateString.toLocal8Bit());
}

void Headers::setHeader(const QString &field, const QStringList &values)
{
    insert(field.toLocal8Bit(), values.join(QLatin1String(", ")).toLocal8Bit());
}

static QList<QByteArray> headerOrder(
{
            // General headers
            "Cache-Control",
            "Connection",
            "Date",
            "Pragma",
            "Trailer",
            "Transfer-Encoding",
            "Upgrade",
            "Via",
            "Warning",
            // Request headers
            "Accept",
            "Accept-Charset",
            "Accept-Encoding",
            "Accept-Language",
            "Authorization",
            "Expect",
            "From",
            "Host",
            "If-Match",
            "If-Modified-Since",
            "If-None-Match",
            "If-Range",
            "If-Unmodified-Since",
            "Max-Forwards",
            "Proxy-Authorization",
            "Range",
            "Referer",
            "TE",
            "User-Agent",
            // Response headers
            "Accept-Ranges",
            "Age",
            "ETag",
            "Location",
            "Proxy-Authenticate",
            "Retry-After",
            "Server",
            "Vary",
            "WWW-Authenticate",
            // Entity headers
            "Allow",
            "Content-Encoding",
            "Content-Language",
            "Content-Length",
            "Content-Location",
            "Content-MD5",
            "Content-Range",
            "Content-Type",
            "Expires",
            "Last-Modified"
        });

bool httpGoodPracticeSort(const HeaderValuePair &pair1, const HeaderValuePair &pair2)
{
    int index1 = headerOrder.indexOf(pair1.first);
    int index2 = headerOrder.indexOf(pair2.first);
    if (index1 != -1 && index2 != -1) {
        // Both items are in the headerOrder list
        return index1 < index2;
    } else if (index1 == -1 && index2 == -1) {
        // Noone of them are int the headerOrder list
        return pair1.first < pair2.first;
    }

    // if the pair1 is in the header list it should go first
    return index1 != -1;
}

QList<HeaderValuePair> Headers::headersForResponse() const
{
    QList<HeaderValuePair> ret;

    QHash<QByteArray, QByteArray>::ConstIterator it = QHash::constBegin();
    while (it != QHash::constEnd()) {
        ret.append(qMakePair(it.key(), it.value()));
        ++it;
    }

    // Sort base on the "good practices" of HTTP RCF
    qSort(ret.begin(), ret.end(), &httpGoodPracticeSort);

    return ret;
}
