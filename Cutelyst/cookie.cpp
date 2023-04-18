/*
 * SPDX-FileCopyrightText: (C) 2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cookie_p.h"

#include <QDateTime>
#include <QHostAddress>
#include <QLocale>
#include <QUrl>

using namespace Cutelyst;

Cookie::Cookie(const QByteArray &name, const QByteArray &value)
    : QNetworkCookie(name, value)
    , d(new CookiePrivate)
{
    qRegisterMetaType<Cookie>();
    qRegisterMetaType<QList<Cookie>>();
}

Cookie::Cookie(const Cookie &other)
    : QNetworkCookie(other)
    , d(other.d)
{
}

Cookie::~Cookie() = default;

Cookie &Cookie::operator=(const Cookie &other)
{
    QNetworkCookie::operator=(other);
    d = other.d;
    return *this;
}

bool Cookie::operator==(const Cookie &other) const
{
    if (QNetworkCookie::operator==(other) && d == other.d) {
        return true;
    }

    return QNetworkCookie::operator==(other) && d->sameSite == other.d->sameSite;
}

Cookie::SameSite Cookie::sameSitePolicy() const
{
    return d->sameSite;
}

void Cookie::setSameSitePolicy(Cookie::SameSite sameSite)
{
    d->sameSite = sameSite;
}

namespace {
QByteArray sameSiteToRawString(Cookie::SameSite samesite)
{
    switch (samesite) {
    case Cookie::SameSite::None:
        return QByteArrayLiteral("None");
    case Cookie::SameSite::Lax:
        return QByteArrayLiteral("Lax");
    case Cookie::SameSite::Strict:
        return QByteArrayLiteral("Strict");
    case Cookie::SameSite::Default:
        break;
    }
    return QByteArray();
}
} // namespace

QByteArray Cookie::toRawForm(RawForm form) const
{
    QByteArray result;
    if (name().isEmpty())
        return result; // not a valid cookie

    result = name();
    result += '=';
    result += value();

    if (form == Full) {
        // same as above, but encoding everything back
        if (isSecure())
            result += "; secure";
        if (isHttpOnly())
            result += "; HttpOnly";
        if (d->sameSite != SameSite::Default) {
            result += "; SameSite=";
            result += sameSiteToRawString(d->sameSite);
        }
        if (!isSessionCookie()) {
            result += "; expires=";
            result += QLocale::c().toString(expirationDate().toUTC(),
                                            QStringLiteral("ddd, dd-MMM-yyyy hh:mm:ss 'GMT"))
                          .toLatin1();
        }
        if (!domain().isEmpty()) {
            result += "; domain=";
            if (domain().startsWith(u'.')) {
                result += '.';
                result += QUrl::toAce(domain().mid(1));
            } else {
                QHostAddress hostAddr(domain());
                if (hostAddr.protocol() == QAbstractSocket::IPv6Protocol) {
                    result += '[';
                    result += domain().toUtf8();
                    result += ']';
                } else {
                    result += QUrl::toAce(domain());
                }
            }
        }
        if (!path().isEmpty()) {
            result += "; path=";
            result += path().toUtf8();
        }
    }
    return result;
}

#ifndef QT_NO_DEBUG_STREAM
QDebug operator<<(QDebug s, const Cutelyst::Cookie &cookie)
{
    QDebugStateSaver saver(s);
    Q_UNUSED(saver)
    s.resetFormat().nospace();
    s << "Cutelyst::Cookie(" << cookie.toRawForm(Cookie::Full) << ')';
    return s;
}
#endif

#include "moc_cookie.cpp"
