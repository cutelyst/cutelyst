/*
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include <QStringList>

using namespace Cutelyst;

Headers::Headers()
{

}

QString Headers::contentEncoding() const
{
    return m_data.value(QStringLiteral("content_encoding"));
}

void Headers::setContentEncoding(const QString &encoding)
{
    m_data.insert(QStringLiteral("content_encoding"), encoding);
}

QString Headers::contentType() const
{
    QString ret;
    const auto it = m_data.constFind(QStringLiteral("content_type"));
    if (it != m_data.constEnd()) {
        const QString ct = it.value();
        ret = ct.mid(0, ct.indexOf(QLatin1Char(';'))).toLower();
    }
    return ret;
}

void Headers::setContentType(const QString &contentType)
{
    m_data.insert(QStringLiteral("content_type"), contentType);
}

QString Headers::contentTypeCharset() const
{
    QString ret;
    const auto it = m_data.constFind(QStringLiteral("content_type"));
    if (it != m_data.constEnd()) {
        const QString contentType = it.value();
        int pos = contentType.indexOf(QLatin1String("charset="), 0, Qt::CaseInsensitive);
        if (pos != -1) {
            int endPos = contentType.indexOf(QLatin1Char(';'), pos);
            ret = contentType.mid(pos + 8, endPos).trimmed().toUpper();
        }
    }

    return ret;
}

void Headers::setContentTypeCharset(const QString &charset)
{
    const auto it = m_data.constFind(QStringLiteral("content_type"));
    if (it == m_data.constEnd() || (it.value().isEmpty() && !charset.isEmpty())) {
        m_data.insert(QStringLiteral("content_type"), QLatin1String("charset=") + charset);
        return;
    }

    QString contentType = it.value();
    int pos = contentType.indexOf(QLatin1String("charset="), 0, Qt::CaseInsensitive);
    if (pos != -1) {
        int endPos = contentType.indexOf(QLatin1Char(';'), pos);
        if (endPos == -1) {
            if (charset.isEmpty()) {
                int lastPos = contentType.lastIndexOf(QLatin1Char(';'), pos);
                if (lastPos == -1) {
                    m_data.remove(QStringLiteral("content_type"));
                    return;
                } else {
                    contentType.remove(lastPos, contentType.length() - lastPos);
                }
            } else {
                contentType.replace(pos + 8, contentType.length() - pos + 8, charset);
            }
        } else {
            contentType.replace(pos + 8, endPos, charset);
        }
    } else if (!charset.isEmpty()) {
        contentType.append(QLatin1String("; charset=") + charset);
    }
    m_data.insert(QStringLiteral("content_type"), contentType);
}

bool Headers::contentIsText() const
{
    return m_data.value(QStringLiteral("content_type")).startsWith(QLatin1String("text/"));
}

bool Headers::contentIsHtml() const
{
    const QString ct = contentType();
    return ct == QLatin1String("text/html") ||
            ct == QLatin1String("application/xhtml+xml") ||
            ct == QLatin1String("application/vnd.wap.xhtml+xml");
}

bool Headers::contentIsXHtml() const
{
    const QString ct = contentType();
    return ct == QLatin1String("application/xhtml+xml") ||
            ct == QLatin1String("application/vnd.wap.xhtml+xml");
}

bool Headers::contentIsXml() const
{
    const QString ct = contentType();
    return ct == QLatin1String("text/xml") ||
            ct == QLatin1String("application/xml") ||
            ct.endsWith(QLatin1String("xml"));
}

qint64 Headers::contentLength() const
{
    return m_data.value(QStringLiteral("content_length")).toLongLong();
}

void Headers::setContentLength(qint64 value)
{
    m_data.insert(QStringLiteral("content_length"), QString::number(value));
}

QString Headers::setDateWithDateTime(const QDateTime &date)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    const QString dt = QLocale::c().toString(date.toUTC(),
                                             QStringLiteral("ddd, dd MMM yyyy hh:mm:ss 'GMT"));
    m_data.insert(QStringLiteral("date"), dt);
    return dt;
}

QDateTime Headers::date()
{
    QDateTime ret;
    auto it = m_data.constFind(QStringLiteral("date"));
    if (it != m_data.constEnd()) {
        const QString date = it.value();

        QDateTime localDT;
        if (date.endsWith(QLatin1String(" GMT"))) {
            localDT = QLocale::c().toDateTime(date.left(date.size() - 4),
                                              QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        } else {
            localDT = QLocale::c().toDateTime(date,
                                              QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        }

        ret = QDateTime(localDT.date(), localDT.time(), Qt::UTC);
    }

    return ret;
}

QString Headers::ifModifiedSince() const
{
    return header(QStringLiteral("if_modified_since"));
}

QDateTime Headers::ifModifiedSinceDateTime() const
{
    QDateTime ret;
    auto it = m_data.constFind(QStringLiteral("if_modified_since"));
    if (it != m_data.constEnd()) {
        const QString ifModifiedStr = it.value();

        QDateTime localDT;
        if (ifModifiedStr.endsWith(QLatin1String(" GMT"))) {
            localDT = QLocale::c().toDateTime(ifModifiedStr.left(ifModifiedStr.size() - 4),
                                              QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        } else {
            localDT = QLocale::c().toDateTime(ifModifiedStr,
                                              QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        }
        ret = QDateTime(localDT.date(), localDT.time(), Qt::UTC);
    }

    return ret;
}

QString Headers::lastModified() const
{
    return m_data.value(QStringLiteral("last_modified"));
}

void Headers::setLastModified(const QString &value)
{
    m_data.insert(QStringLiteral("last_modified"), value);
}

QString Headers::setLastModified(const QDateTime &lastModified)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    const auto dt = QLocale::c().toString(lastModified.toUTC(),
                                          QStringLiteral("ddd, dd MMM yyyy hh:mm:ss 'GMT"));
    setLastModified(dt);
    return dt;
}

QString Headers::server() const
{
    return m_data.value(QStringLiteral("server"));
}

void Headers::setServer(const QString &value)
{
    m_data.insert(QStringLiteral("server"), value);
}

QString Headers::connection() const
{
    return m_data.value(QStringLiteral("connection"));
}

QString Headers::host() const
{
    return m_data.value(QStringLiteral("host"));
}

QString Headers::userAgent() const
{
    return m_data.value(QStringLiteral("user_agent"));
}

QString Headers::referer() const
{
    return m_data.value(QStringLiteral("referer"));
}

void Headers::setReferer(const QString &uri)
{
    int fragmentPos = uri.indexOf(QLatin1Char('#'));
    if (fragmentPos != -1) {
        // Strip fragment per RFC 2616, section 14.36.
        m_data.insert(QStringLiteral("referer"), uri.mid(0, fragmentPos));
    } else {
        m_data.insert(QStringLiteral("referer"), uri);
    }
}

void Headers::setWwwAuthenticate(const QString &value)
{
    m_data.insert(QStringLiteral("www_authenticate"), value);
}

void Headers::setProxyAuthenticate(const QString &value)
{
    m_data.insert(QStringLiteral("proxy_authenticate"), value);
}

QString Headers::authorization() const
{
    return m_data.value(QStringLiteral("authorization"));
}

QString Headers::authorizationBasic() const
{
    return QString::fromLatin1(HeadersPrivate::decodeBasicAuth(authorization()));
}

QPair<QString, QString> Headers::authorizationBasicPair() const
{
    return HeadersPrivate::decodeBasicAuthPair(authorization());
}

QString Headers::setAuthorizationBasic(const QString &username, const QString &password)
{
    QString ret;
    if (username.contains(QLatin1Char(':'))) {
        qCWarning(CUTELYST_CORE) << "Headers::Basic authorization user name can't contain ':'";
        return ret;
    }

    const QString result = username + QLatin1Char(':') + password;
    ret = QStringLiteral("Basic ") + QString::fromLatin1(result.toLatin1().toBase64());
    m_data.insert(QStringLiteral("authorization"), ret);
    return ret;
}

QString Headers::proxyAuthorization() const
{
    return m_data.value(QStringLiteral("proxy_authorization"));
}

QString Headers::proxyAuthorizationBasic() const
{
    return QString::fromLatin1(HeadersPrivate::decodeBasicAuth(proxyAuthorization()));
}

QPair<QString, QString> Headers::proxyAuthorizationBasicPair() const
{
    return HeadersPrivate::decodeBasicAuthPair(proxyAuthorization());
}

QString Headers::header(const QString &field) const
{
    return m_data.value(HeadersPrivate::normalizeHeaderKey(field));
}

QString Headers::header(const QString &field, const QString &defaultValue) const
{
    return m_data.value(HeadersPrivate::normalizeHeaderKey(field), defaultValue);
}

void Headers::setHeader(const QString &field, const QString &value)
{
    m_data.insert(HeadersPrivate::normalizeHeaderKey(field), value);
}

void Headers::setHeader(const QString &field, const QStringList &values)
{
    setHeader(field, values.join(QStringLiteral(", ")));
}

void Headers::pushHeader(const QString &field, const QString &value)
{
    m_data.insertMulti(HeadersPrivate::normalizeHeaderKey(field), value);
}

void Headers::pushHeader(const QString &field, const QStringList &values)
{
    m_data.insertMulti(HeadersPrivate::normalizeHeaderKey(field), values.join(QStringLiteral(", ")));
}

void Headers::removeHeader(const QString &field)
{
    m_data.remove(HeadersPrivate::normalizeHeaderKey(field));
}

bool Headers::contains(const QString &field)
{
    return m_data.contains(HeadersPrivate::normalizeHeaderKey(field));
}

QString &Headers::operator[](const QString &key)
{
    return m_data[key];
}

const QString Headers::operator[](const QString &key) const
{
    return m_data[key];
}

QString HeadersPrivate::normalizeHeaderKey(const QString &field)
{
    QString key = field;
    int i = 0;
    while (i < key.size()) {
        QCharRef c = key[i];
        if (c.isSpace()) {
            key.remove(i, 1);
            continue;
        } else if (c == QLatin1Char('-')) {
            c = QLatin1Char('_');
        } else {
            c = c.toLower();
        }
        ++i;
    }
    return key;
}

QByteArray HeadersPrivate::decodeBasicAuth(const QString &auth)
{
    QByteArray ret;
    if (!auth.isEmpty() && auth.startsWith(QLatin1String("Basic "))) {
        int pos = auth.lastIndexOf(QLatin1Char(' '));
        if (pos != -1) {
            ret = QByteArray::fromBase64(auth.mid(pos).toLatin1());
        }
    }
    return ret;
}

QPair<QString, QString> HeadersPrivate::decodeBasicAuthPair(const QString &auth)
{
    QPair<QString, QString> ret;
    const QByteArray authorization = decodeBasicAuth(auth);
    if (!authorization.isEmpty()) {
        int pos = authorization.indexOf(':');
        if (pos == -1) {
            ret.first = QString::fromLatin1(authorization);
        } else {
            ret = qMakePair(QString::fromLatin1(authorization.left(pos)),
                            QString::fromLatin1(authorization.mid(pos + 1)));
        }
    }
    return ret;
}
