/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "headers.h"

#include "common.h"
#include "engine.h"

#include <QStringList>

using namespace Cutelyst;

inline QByteArray decodeBasicAuth(const QByteArray &auth);
inline Headers::Authorization decodeBasicAuthPair(const QByteArray &auth);

namespace {
QVector<Headers::HeaderKeyValue>::const_iterator
    findHeaderConst(const QVector<Headers::HeaderKeyValue> &headers, QByteArrayView key) noexcept
{
    auto matchKey = [key](Headers::HeaderKeyValue entry) {
        return key.compare(entry.key, Qt::CaseInsensitive) == 0;
    };
    return std::find_if(headers.cbegin(), headers.cend(), matchKey);
}
} // namespace

Headers::Headers(const Headers &other) noexcept
    : m_data(other.m_data)
{
}

QByteArray Headers::contentDisposition() const noexcept
{
    return header("Content-Disposition");
}

void Headers::setCacheControl(const QByteArray &value)
{
    setHeader("Cache-Control"_qba, value);
}

void Headers::setContentDisposition(const QByteArray &contentDisposition)
{
    setHeader("Content-Disposition"_qba, contentDisposition);
}

void Headers::setContentDispositionAttachment(const QByteArray &filename)
{
    if (filename.isEmpty()) {
        setContentDisposition("attachment");
    } else {
        setContentDisposition("attachment; filename=\"" + filename + '"');
    }
}

QByteArray Headers::contentEncoding() const noexcept
{
    return header("Content-Encoding");
}

void Headers::setContentEncoding(const QByteArray &encoding)
{
    setHeader("Content-Encoding"_qba, encoding);
}

QByteArray Headers::contentType() const
{
    QByteArray ret = header("Content-Type");
    if (!ret.isEmpty()) {
        ret = ret.mid(0, ret.indexOf(';')).toLower();
    }
    return ret;
}

void Headers::setContentType(const QByteArray &contentType)
{
    setHeader("Content-Type"_qba, contentType);
}

QByteArray Headers::contentTypeCharset() const
{
    QByteArray ret;
    const QByteArray contentType = header("Content-Type");
    if (!contentType.isEmpty()) {
        int pos = contentType.indexOf("charset=", 0);
        if (pos != -1) {
            int endPos = contentType.indexOf(u';', pos);
            ret        = contentType.mid(pos + 8, endPos).trimmed().toUpper();
        }
    }

    return ret;
}

void Headers::setContentTypeCharset(const QByteArray &charset)
{
    auto result = findHeaderConst(m_data, "Content-Type");
    if (result == m_data.end() || (result->value.isEmpty() && !charset.isEmpty())) {
        setContentType("charset=" + charset);
        return;
    }

    QByteArray contentType = result->value;
    int pos                = contentType.indexOf("charset=", 0);
    if (pos != -1) {
        int endPos = contentType.indexOf(';', pos);
        if (endPos == -1) {
            if (charset.isEmpty()) {
                int lastPos = contentType.lastIndexOf(';', pos);
                if (lastPos == -1) {
                    removeHeader("Content-Type");
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
        contentType.append("; charset=" + charset);
    }
    setContentType(contentType);
}

bool Headers::contentIsText() const
{
    return header("Content-Type").startsWith("text/");
}

bool Headers::contentIsHtml() const
{
    const QByteArray ct = contentType();
    return ct.compare("text/html") == 0 || ct.compare("application/xhtml+xml") == 0 ||
           ct.compare("application/vnd.wap.xhtml+xml") == 0;
}

bool Headers::contentIsXHtml() const
{
    const QByteArray ct = contentType();
    return ct.compare("application/xhtml+xml") == 0 ||
           ct.compare("application/vnd.wap.xhtml+xml") == 0;
}

bool Headers::contentIsXml() const
{
    const QByteArray ct = contentType();
    return ct.compare("text/xml") == 0 || ct.compare("application/xml") == 0 || ct.endsWith("xml");
}

bool Headers::contentIsJson() const
{
    auto value = header("Content-Type");
    if (!value.isEmpty()) {
        return value.compare("application/json") == 0;
    }
    return false;
}

qint64 Headers::contentLength() const
{
    auto value = header("Content-Length");
    if (!value.isEmpty()) {
        return value.toLongLong();
    }
    return -1;
}

void Headers::setContentLength(qint64 value)
{
    setHeader("Content-Length"_qba, QByteArray::number(value));
}

QByteArray Headers::setDateWithDateTime(const QDateTime &date)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    QByteArray dt =
        QLocale::c().toString(date.toUTC(), u"ddd, dd MMM yyyy hh:mm:ss 'GMT").toLatin1();
    setHeader("Date"_qba, dt);
    return dt;
}

QDateTime Headers::date() const
{
    QDateTime ret;
    auto value = header("Date");
    if (!value.isEmpty()) {
        if (value.endsWith(" GMT")) {
            ret = QLocale::c().toDateTime(QString::fromLatin1(value.left(value.size() - 4)),
                                          QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        } else {
            ret = QLocale::c().toDateTime(QString::fromLatin1(value),
                                          QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        }
        ret.setTimeSpec(Qt::UTC);
    }

    return ret;
}

QByteArray Headers::ifModifiedSince() const noexcept
{
    return header("If-Modified-Since");
}

QDateTime Headers::ifModifiedSinceDateTime() const
{
    QDateTime ret;
    auto value = header("If-Modified-Since");
    if (!value.isEmpty()) {
        if (value.endsWith(" GMT")) {
            ret = QLocale::c().toDateTime(QString::fromLatin1(value.left(value.size() - 4)),
                                          QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        } else {
            ret = QLocale::c().toDateTime(QString::fromLatin1(value),
                                          QStringLiteral("ddd, dd MMM yyyy hh:mm:ss"));
        }
        ret.setTimeSpec(Qt::UTC);
    }

    return ret;
}

bool Headers::ifModifiedSince(const QDateTime &lastModified) const
{
    auto value = header("If-Modified-Since");
    if (!value.isEmpty()) {
        return value != QLocale::c()
                            .toString(lastModified.toUTC(), u"ddd, dd MMM yyyy hh:mm:ss 'GMT")
                            .toLatin1();
    }
    return true;
}

bool Headers::ifMatch(const QByteArray &etag) const
{
    auto value = header("If-Match");
    if (!value.isEmpty()) {
        const auto clientETag = QByteArrayView(value);
        return clientETag.sliced(1, clientETag.size() - 2) == etag ||
               clientETag.sliced(3, clientETag.size() - 4) == etag; // Weak ETag
    }
    return true;
}

bool Headers::ifNoneMatch(const QByteArray &etag) const
{
    auto value = header("If-None-Match");
    if (!value.isEmpty()) {
        const auto clientETag = QByteArrayView(value);
        return clientETag.sliced(1, clientETag.size() - 2) == etag ||
               clientETag.sliced(3, clientETag.size() - 4) == etag; // Weak ETag
    }
    return false;
}

void Headers::setETag(const QByteArray &etag)
{
    setHeader("ETag"_qba, '"' + etag + '"');
}

QByteArray Headers::lastModified() const noexcept
{
    return header("Last-Modified");
}

void Headers::setLastModified(const QByteArray &value)
{
    setHeader("Last-Modified"_qba, value);
}

QString Headers::setLastModified(const QDateTime &lastModified)
{
    // ALL dates must be in GMT timezone http://www.w3.org/Protocols/rfc2616/rfc2616-sec3.html
    // and follow RFC 822
    auto dt = QLocale::c().toString(lastModified.toUTC(), u"ddd, dd MMM yyyy hh:mm:ss 'GMT");
    setLastModified(dt.toLatin1());
    return dt;
}

QByteArray Headers::server() const noexcept
{
    return header("Server");
}

void Headers::setServer(const QByteArray &value)
{
    setHeader("Server"_qba, value);
}

QByteArray Headers::connection() const noexcept
{
    return header("Connection");
}

QByteArray Headers::host() const noexcept
{
    return header("Host");
}

QByteArray Headers::userAgent() const noexcept
{
    return header("User-Agent");
}

QByteArray Headers::referer() const noexcept
{
    return header("Referer");
}

void Headers::setReferer(const QByteArray &uri)
{
    int fragmentPos = uri.indexOf('#');
    if (fragmentPos != -1) {
        // Strip fragment per RFC 2616, section 14.36.
        setHeader("Referer"_qba, uri.mid(0, fragmentPos));
    } else {
        setHeader("Referer"_qba, uri);
    }
}

void Headers::setWwwAuthenticate(const QByteArray &value)
{
    setHeader("Www-Authenticate"_qba, value);
}

void Headers::setProxyAuthenticate(const QByteArray &value)
{
    setHeader("Proxy-Authenticate"_qba, value);
}

QByteArray Headers::authorization() const noexcept
{
    return header("Authorization");
}

QByteArray Headers::authorizationBearer() const
{
    QByteArray ret;
    auto auth = authorization();
    int pos   = auth.indexOf("Bearer ");
    if (pos != -1) {
        pos += 7;
        ret = auth.mid(pos, auth.indexOf(',', pos) - pos);
    }
    return ret;
}

QByteArray Headers::authorizationBasic() const
{
    return decodeBasicAuth(authorization());
}

Headers::Authorization Headers::authorizationBasicObject() const
{
    return decodeBasicAuthPair(authorization());
}

QByteArray Headers::setAuthorizationBasic(const QString &username, const QString &password)
{
    QByteArray ret;
    if (username.contains(u':')) {
        qCWarning(CUTELYST_CORE) << "Headers::Basic authorization user name can't contain ':'";
        return ret;
    }

    const QString result = username + u':' + password;
    ret                  = "Basic " + result.toLatin1().toBase64();
    setHeader("Authorization"_qba, ret);
    return ret;
}

QByteArray Headers::proxyAuthorization() const noexcept
{
    return header("Proxy-Authorization");
}

QByteArray Headers::proxyAuthorizationBasic() const
{
    return decodeBasicAuth(proxyAuthorization());
}

Headers::Authorization Headers::proxyAuthorizationBasicObject() const
{
    return decodeBasicAuthPair(proxyAuthorization());
}

QByteArray Headers::header(QByteArrayView key) const noexcept
{
    if (auto result = findHeaderConst(m_data, key); result != m_data.end()) {
        return result->value;
    }
    return {};
}

QString Headers::headerAsString(QByteArrayView key) const
{
    return QString::fromLatin1(header(key));
}

QByteArray Headers::header(QByteArrayView key, const QByteArray &defaultValue) const noexcept
{
    if (auto result = findHeaderConst(m_data, key); result != m_data.end()) {
        return result->value;
    }
    return defaultValue;
}

QString Headers::headerAsString(QByteArrayView key, const QByteArray &defaultValue) const
{
    return QString::fromLatin1(header(key, defaultValue));
}

QByteArrayList Headers::headers(QByteArrayView key) const
{
    QByteArrayList ret;
    for (auto result = findHeaderConst(m_data, key); result != m_data.end(); ++result) {
        ret.append(result->value);
    }
    return ret;
}

QStringList Headers::headersAsStrings(QByteArrayView key) const
{
    QStringList ret;
    for (auto result = findHeaderConst(m_data, key); result != m_data.end(); ++result) {
        ret.append(QString::fromLatin1(result->value));
    }
    return ret;
}

void Headers::setHeader(const QByteArray &key, const QByteArray &value)
{
    auto matchKey = [key](Headers::HeaderKeyValue entry) {
        return key.compare(entry.key, Qt::CaseInsensitive) == 0;
    };

    if (auto result = std::find_if(m_data.begin(), m_data.end(), matchKey);
        result != m_data.end()) {
        result->value = value;
        ++result;

        m_data.erase(std::remove_if(result, m_data.end(), matchKey), m_data.end());
    } else {
        m_data.emplace_back(HeaderKeyValue{key, value});
    }
}

void Headers::setHeader(const QByteArray &field, const QByteArrayList &values)
{
    setHeader(field, values.join(", "));
}

void Headers::pushHeader(const QByteArray &key, const QByteArray &value)
{
    m_data.push_back({key, value});
}

void Headers::pushHeader(const QByteArray &key, const QByteArrayList &values)
{
    m_data.push_back({key, values.join(", ")});
}

void Headers::removeHeader(QByteArrayView key)
{
    m_data.removeIf(
        [key](HeaderKeyValue entry) { return key.compare(entry.key, Qt::CaseInsensitive) == 0; });
}

bool Headers::contains(QByteArrayView key) const noexcept
{
    auto result = findHeaderConst(m_data, key);
    return result != m_data.end();
}

QByteArrayList Headers::keys() const
{
    QByteArrayList ret;

    for (const auto &header : m_data) {
        bool found = false;
        for (const auto &key : ret) {
            if (header.key.compare(key, Qt::CaseInsensitive) == 0) {
                found = true;
                break;
            }
        }

        if (!found) {
            ret.append(header.key);
        }
    }

    return ret;
}

QByteArray Headers::operator[](QByteArrayView key) const noexcept
{
    return header(key);
}

bool Headers::operator==(const Headers &other) const noexcept
{
    const auto otherData = other.data();
    if (m_data.size() != otherData.size()) {
        return false;
    }

    for (const auto &myValue : m_data) {
        if (!other.data().contains(myValue)) {
            return false;
        }
    }

    return true;
}

QByteArray decodeBasicAuth(const QByteArray &auth)
{
    QByteArray ret;
    int pos = auth.indexOf("Basic ");
    if (pos != -1) {
        pos += 6;
        ret = auth.mid(pos, auth.indexOf(',', pos) - pos);
        ret = QByteArray::fromBase64(ret);
    }
    return ret;
}

Headers::Authorization decodeBasicAuthPair(const QByteArray &auth)
{
    Headers::Authorization ret;
    const QByteArray authorization = decodeBasicAuth(auth);
    if (!authorization.isEmpty()) {
        int pos = authorization.indexOf(':');
        if (pos == -1) {
            ret.user = QString::fromLatin1(authorization);
        } else {
            ret.user     = QString::fromLatin1(authorization.left(pos));
            ret.password = QString::fromLatin1(authorization.mid(pos + 1));
        }
    }
    return ret;
}

QDebug operator<<(QDebug debug, const Headers &headers)
{
    const auto data       = headers.data();
    const bool oldSetting = debug.autoInsertSpaces();
    debug.nospace() << "Headers[";
    for (auto it = data.begin(); it != data.end(); ++it) {
        debug << '(' << it->key + '=' + it->value << ')';
    }
    debug << ']';
    debug.setAutoInsertSpaces(oldSetting);
    return debug.maybeSpace();
}
