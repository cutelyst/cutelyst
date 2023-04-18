/*
 * SPDX-FileCopyrightText: (C) 2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYST_COOKIE_H
#define CUTELYST_COOKIE_H

#include <Cutelyst/cutelyst_global.h>

#include <QNetworkCookie>
#include <QObject>

namespace Cutelyst {

class CookiePrivate;
/*!
 * \class Cookie cookie.h Cutelyst/Cookie
 * \brief The %Cutelyst %Cookie
 *
 * This is an extension of QNetworkCookie that adds support for the \a SameSite property.
 * QNetworkCookie added support for the SameSite property in Qt 6.1. So, if you are only
 * using Qt 6.1 or newer, please use QNetworkCookie and the methods that take it as argument
 * instead.
 *
 * \since Cutelyst 3.8.0
 */
class CUTELYST_LIBRARY Cookie : public QNetworkCookie
{
    Q_GADGET
public:
    enum class SameSite {
        Default, /**< SameSite is not set. Can be interpreted as None or Lax by the browser. */
        None,    /**< Cookies can be sent in all contexts. This used to be default, but recent browsers made Lax default, and will now require the cookie to be both secure and to set SameSite=None. */
        Lax,     /**< Cookies are sent on first party requests and GET requests initiated by third party website. This is the default in modern browsers (since mid 2020). */
        Strict   /**< Cookies will only be sent in a first-party context. */
    };
    Q_ENUM(SameSite)

    /*!
     * Create a new %Cookie object, initializing the cookie name to \a name and its value to \a value.
     *
     * A cookie is only valid if it has a name. However, the value is opaque to the application and being empty may have significance to the remote server.
     */
    explicit Cookie(const QByteArray &name = QByteArray(), const QByteArray &value = QByteArray());
    /*!
     * Creates a new %Cookie object by copying the contents of other.
     */
    Cookie(const Cookie &other);
    /*!
     * Destroys this %Cookie object.
     */
    ~Cookie();
    /*!
     * Move assigns the contents of the %Cookie object \a other to this object.
     */
    Cookie &operator=(Cookie &&other) noexcept
    {
        swap(other);
        return *this;
    }
    /*!
     * Copies the contents of the %Cookie object \a other to this object.
     */
    Cookie &operator=(const Cookie &other);

    /*!
     * Swaps this cookie with other. This function is very fast and never fails.
     */
    void swap(Cookie &other) noexcept
    {
        QNetworkCookie::swap(other);
        d.swap(other.d);
    }

    /*!
     * Returns true if this cookie is equal to \a other. This function only returns true if all fields of the cookie are the same.
     *
     * However, in some contexts, two cookies of the same name could be considered equal.
     */
    bool operator==(const Cookie &other) const;
    /*!
     * Returns true if this cookie is not equal to \a other.
     */
    inline bool operator!=(const Cookie &other) const
    {
        return !(*this == other);
    }

    /*!
     * Returns the "SameSite" option if specified in the cookie string, SameSite::Default if not present.
     */
    SameSite sameSitePolicy() const;
    /*!
     * Sets the "SameSite" option of this cookie to \a sameSite.
     */
    void setSameSitePolicy(SameSite sameSite);

    /*!
     * Returns the raw form of this %Cookie. The QByteArray returned by this function is suitable for an HTTP header,
     * either in a server response (the Set-Cookie header) or the client request (the Cookie header).
     * You can choose from one of two formats, using \a form.
     */
    QByteArray toRawForm(RawForm form = Full) const;

private:
    QSharedDataPointer<CookiePrivate> d;
};

} // namespace Cutelyst

Q_DECLARE_TYPEINFO(Cutelyst::Cookie, Q_MOVABLE_TYPE);

#ifndef QT_NO_DEBUG_STREAM
class QDebug;
CUTELYST_LIBRARY QDebug operator<<(QDebug, const Cutelyst::Cookie &);
#endif

#endif // CUTELYST_COOKIE_H
