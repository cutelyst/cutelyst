/*
 * Copyright (C) 2014-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include "headers.h"

#include "common.h"

#include "engine.h"

#include <QStringList>

using namespace Cutelyst;

inline QString normalizeHeaderKey(const QString &field);
inline QByteArray decodeBasicAuth(const QString &auth);
inline std::pair<QString, QString> decodeBasicAuthPair(const QString &auth);

static const QString _contentType(QLatin1String("CONTENT_TYPE"));
static const QString _contentDisposition(QLatin1String("CONTENT_DISPOSITION"));
static const QString _contentEncoding(QLatin1String("CONTENT_ENCODING"));
static const QString _contentLength(QLatin1String("CONTENT_LENGTH"));
static const QString _date(QLatin1String("DATE"));
static const QString _ifModifiedSince(QLatin1String("IF_MODIFIED_SINCE"));
static const QString _lastModified(QLatin1String("LAST_MODIFIED"));
static const QString _server(QLatin1String("SERVER"));
static const QString _referer(QLatin1String("REFERER"));
static const QString _authorization(QLatin1String("AUTHORIZATION"));

static const QString _formatDateTime(QLatin1String("ddd, dd MMM yyyy hh:mm:ss"));
static const QString _formatDateTimeGMT(QLatin1String("ddd, dd MMM yyyy hh:mm:ss 'GMT"));
static const QString _joinComaSpace(QLatin1String(", "));

Headers::Headers()
{

}

QString Headers::contentDisposition() const
{
    return m_data.value(_contentDisposition);
}

void Headers::setContentDisposition(const QString &contentDisposition)
{
    m_data.insert(_contentDisposition, contentDisposition);
}

void Headers::setContentDispositionAttachment(const QString &filename)
{
    if (filename.isEmpty()) {
        setContentDisposition(QStringLiteral("attachment"));
    } else {
        setContentDisposition(QLatin1String("attachment; filename=\"") + filename + QLatin1Char('"'));
    }
}

QString Headers::contentEncoding() const
{
    return m_data.value(_contentEncoding);
}

void Headers::setContentEncoding(const QString &encoding)
{
    m_data.insert(_contentEncoding, encoding);
}

QString Headers::contentType() const
{
    QString ret;
    const auto it = m_data.constFind(_contentType);
    if (it != m_data.constEnd()) {
        const QString &ct = it.value();
        ret = ct.mid(0, ct.indexOf(QLatin1Char(';'))).toLower();
    }
    return ret;
}

void Headers::setContentType(const QString &contentType)
{
    m_data.insert(_contentType, contentType);
}

QString Headers::contentTypeCharset() const
{
    QString ret;
    const auto it = m_data.constFind(_contentType);
    if (it != m_data.constEnd()) {
        const QString &contentType = it.value();
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
    const auto it = m_data.constFind(_contentType);
    if (it == m_data.constEnd() || (it.value().isEmpty() && !charset.isEmpty())) {
        m_data.insert(_contentType, QLatin1String("charset=") + charset);
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
                    m_data.remove(_contentType);
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
    m_data.insert(_contentType, contentType);
}

bool Headers::contentIsText() const
{
    return m_data.value(_contentType).startsWith(QLatin1String("text/"));
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
    auto it = m_data.constFind(_contentLength);
    if (it != m_data.constEnd()) {
        return it.value().toLongLong();
    }
    return -1;
}

void Headers::setContentLength(qint64 value)
{
    m_data.insert(_contentLength, QString::number(value));
}

QString Headers::setDateWithDateTime(const QDateTime &date)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    const QString dt = QLocale::c().toString(date.toUTC(),
                                             _formatDateTimeGMT);
    m_data.insert(_date, dt);
    return dt;
}

QDateTime Headers::date() const
{
    QDateTime ret;
    auto it = m_data.constFind(_date);
    if (it != m_data.constEnd()) {
        const QString &date = it.value();

        if (date.endsWith(QLatin1String(" GMT"))) {
            ret = QLocale::c().toDateTime(date.left(date.size() - 4),
                                          _formatDateTime);
        } else {
            ret = QLocale::c().toDateTime(date,
                                          _formatDateTime);
        }
        ret.setTimeSpec(Qt::UTC);
    }

    return ret;
}

QString Headers::ifModifiedSince() const
{
    return header(_ifModifiedSince);
}

QDateTime Headers::ifModifiedSinceDateTime() const
{
    QDateTime ret;
    auto it = m_data.constFind(_ifModifiedSince);
    if (it != m_data.constEnd()) {
        const QString &ifModifiedStr = it.value();

        if (ifModifiedStr.endsWith(QLatin1String(" GMT"))) {
            ret = QLocale::c().toDateTime(ifModifiedStr.left(ifModifiedStr.size() - 4),
                                          _formatDateTime);
        } else {
            ret = QLocale::c().toDateTime(ifModifiedStr,
                                          _formatDateTime);
        }
        ret.setTimeSpec(Qt::UTC);
    }

    return ret;
}

QString Headers::lastModified() const
{
    return m_data.value(_lastModified);
}

void Headers::setLastModified(const QString &value)
{
    m_data.insert(_lastModified, value);
}

QString Headers::setLastModified(const QDateTime &lastModified)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    const auto dt = QLocale::c().toString(lastModified.toUTC(),
                                          _formatDateTimeGMT);
    setLastModified(dt);
    return dt;
}

QString Headers::server() const
{
    return m_data.value(_server);
}

void Headers::setServer(const QString &value)
{
    m_data.insert(_server, value);
}

QString Headers::connection() const
{
    return m_data.value(QStringLiteral("CONNECTION"));
}

QString Headers::host() const
{
    return m_data.value(QStringLiteral("HOST"));
}

QString Headers::userAgent() const
{
    return m_data.value(QStringLiteral("USER_AGENT"));
}

QString Headers::referer() const
{
    return m_data.value(_referer);
}

void Headers::setReferer(const QString &uri)
{
    int fragmentPos = uri.indexOf(QLatin1Char('#'));
    if (fragmentPos != -1) {
        // Strip fragment per RFC 2616, section 14.36.
        m_data.insert(_referer, uri.mid(0, fragmentPos));
    } else {
        m_data.insert(_referer, uri);
    }
}

void Headers::setWwwAuthenticate(const QString &value)
{
    m_data.insert(QStringLiteral("WWW_AUTHENTICATE"), value);
}

void Headers::setProxyAuthenticate(const QString &value)
{
    m_data.insert(QStringLiteral("PROXY_AUTHENTICATE"), value);
}

QString Headers::authorization() const
{
    return m_data.value(_authorization);
}

QString Headers::authorizationBasic() const
{
    return QString::fromLatin1(decodeBasicAuth(authorization()));
}

std::pair<QString, QString> Headers::authorizationBasicPair() const
{
    return decodeBasicAuthPair(authorization());
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
    m_data.insert(_authorization, ret);
    return ret;
}

QString Headers::proxyAuthorization() const
{
    return m_data.value(QStringLiteral("PROXY_AUTHORIZATION"));
}

QString Headers::proxyAuthorizationBasic() const
{
    return QString::fromLatin1(decodeBasicAuth(proxyAuthorization()));
}

std::pair<QString, QString> Headers::proxyAuthorizationBasicPair() const
{
    return decodeBasicAuthPair(proxyAuthorization());
}

QString Headers::header(const QString &field) const
{
    return m_data.value(normalizeHeaderKey(field));
}

QString Headers::header(const QString &field, const QString &defaultValue) const
{
    return m_data.value(normalizeHeaderKey(field), defaultValue);
}

void Headers::setHeader(const QString &field, const QString &value)
{
    m_data.insert(normalizeHeaderKey(field), value);
}

void Headers::setHeader(const QString &field, const QStringList &values)
{
    setHeader(field, values.join(_joinComaSpace));
}

void Headers::pushHeader(const QString &field, const QString &value)
{
    m_data.insertMulti(normalizeHeaderKey(field), value);
}

void Headers::pushHeader(const QString &field, const QStringList &values)
{
    m_data.insertMulti(normalizeHeaderKey(field), values.join(_joinComaSpace));
}

void Headers::removeHeader(const QString &field)
{
    m_data.remove(normalizeHeaderKey(field));
}

bool Headers::contains(const QString &field)
{
    return m_data.contains(normalizeHeaderKey(field));
}

QString &Headers::operator[](const QString &key)
{
    return m_data[key];
}

const QString Headers::operator[](const QString &key) const
{
    return m_data[key];
}

QString normalizeHeaderKey(const QString &field)
{
    QString key = field;
    int i = 0;
    while (i < key.size()) {
        QCharRef c = key[i];
        if (c.isLetter()) {
            if (c.isLower()) {
                c = c.toUpper();
            }
        } else if (c == QLatin1Char('-')) {
            c = QLatin1Char('_');
        }
        ++i;
    }
    return key;
}

QByteArray decodeBasicAuth(const QString &auth)
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

std::pair<QString, QString> decodeBasicAuthPair(const QString &auth)
{
    std::pair<QString, QString> ret;
    const QByteArray authorization = decodeBasicAuth(auth);
    if (!authorization.isEmpty()) {
        int pos = authorization.indexOf(':');
        if (pos == -1) {
            ret.first = QString::fromLatin1(authorization);
        } else {
            ret = { QString::fromLatin1(authorization.left(pos)),
                    QString::fromLatin1(authorization.mid(pos + 1)) };
        }
    }
    return ret;
}

QDebug operator<<(QDebug debug, const Headers &headers)
{
    const QHash<QString, QString> data = headers.data();
    const bool oldSetting = debug.autoInsertSpaces();
    debug.nospace() << "Headers(";
    for (auto it = data.constBegin();
         it != data.constEnd(); ++it) {
        debug << '(' << Engine::camelCaseHeader(it.key()) + QLatin1Char('=') + it.value() << ')';
    }
    debug << ')';
    debug.setAutoInsertSpaces(oldSetting);
    return debug.maybeSpace();
}
