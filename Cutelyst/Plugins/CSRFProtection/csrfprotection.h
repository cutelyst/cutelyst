/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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

#ifndef CSRFPROTECTION_H
#define CSRFPROTECTION_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Plugin>

namespace Cutelyst {

class Context;
class CSRFProtectionPrivate;

/**
 * @class Cutelyst::CSRFProtection csrfprotection.h Cutelyst/Plugins/CSRFProtection/CSRFProtection
 * @brief Protect input forms against Cross Site %Request Forgery (CSRF/XSRF) attacks.
 *
 * The CSRFProtection plugin implements a synchronizer token pattern (STP) to protect input forms against
 * <A HREF="https://en.wikipedia.org/wiki/Cross-site_request_forgery">Cross Site %Request Forgery (CSRF/XSRF) attacks</A>.
 * This type of attack occurs when a malicious website contains a link, a form button or some JavaScript
 * that is intended to perform some action on your website, using the credentials of a logged-in user who
 * visits the malicious site in their browser.
 *
 * The first defense against CSRF attacks is to ensure that GET requests (and other @a safe methods, as
 * defined by <A HREF="https://tools.ietf.org/html/rfc7231.html#section-4.2.1">RFC 7231</A>)
 * are side effect free. Requests via @a unsafe methods, such as POST, PUT, and DELETE, can then be
 * protected by this plugin.
 *
 * This plugin has been inspired by the <A HREf="https://docs.djangoproject.com/en/1.11/ref/csrf/">CSRF protection of the Django web framework</A>.
 *
 * <H3>Usage</H3>
 *
 * For general usage and to protect any unsafe action, simply add the plugin to your application.
 * @code{.cpp}
 * #include <Cutelyst/Plugins/CSRFProtection/CSRFProtection>
 *
 * bool MyCutelystApp::init()
 * {
 *     // other initialization stuff
 *
 *     auto csrfProtect = new CSRFProtection(this);
 *     // optional you can exempt complete namespaces from the protection
 *     csrfProtect->setExemptedNamespaces({QStringLiteral("foo")});
 *
 *     // more initialization stuff
 * }
 * @endcode
 *
 * To exempt an action from CSRF protection, add <CODE>:CSRFExempt</CODE> to the attributes.
 * @code{.cpp}
 * C_ATTR(index, :Path :AutoArgs :CSRFExempt)
 * void index(Context *c);
 * @endcode
 *
 * If you have optionally exempted complete namespaces from the CSRF protection, you can require
 * the protection for single namespace members by adding <CODE>:CSRFRequire</CODE> to the attributes.
 *
 * @code{.cpp}
 * class Foo : public Cutelyst::Controller
 * {
 *     Q_OBJECT
 *     C_NAMESPACE("foo")
 * public:
 *     C_ATTR(index, :Path :AutoArgs)
 *     void index(Context *c);
 *
 *     C_ATTR(edit, :Path :CSRFRequire)
 *     void edit(Context *c);
 * }
 * @endcode
 *
 * In your Grantlee template you should than use the <CODE>{% c_csrf_token %}</CODE> tag in your forms
 * to add a hidden input field that contains the CSRF protection token.
 *
 * @code{.html}
 * <form method="post">
 *     {% c_csrf_token %}
 *     <input type="text" name="username">
 *     <input type="password" name="password">
 *     <button type="submit">Login</button>
 * </form>
 * @endcode
 *
 * You can optionally get the token by using CSRFProtection::getToken() static function if you
 * want to add it to the stash or want to use it elsewhere. For example, if you don't have a form
 * on your site but want to use the token for an AJAX request and want to add it therefore into a
 * meta tag or something like that.
 *
 * <H4>Handling failed checks</H4>
 *
 * If the CSRF protection check fails, the return code will be set to 403 - Forbidden and an error message
 * will be set to the stash key defined by setErrorMsgStashKey(). You can set a default action the application
 * should detach to if the check failed via setDefaultDetachTo(), optionally there is the attribute <CODE>:CSRFDetachTo</CODE>
 * that can be used to define a detach to action per method. If the detach to action is not set or could not be found
 * a default error page will be generated.
 *
 * @code{.cpp}
 * bool MyCutelystApp::init()
 * {
 *     auto csrf = new CSRFProtection(this);
 *     csrf->setDefaultDetachTo(QStringLiteral("csrffailed"));
 * }
 *
 * class Foo : public Cutelyst::Controller
 * {
 *     C_ATTR(foo, :Local :CSRFDetachTo(csrfDenied))
 *     void foo(Context *c);
 *
 *     C_ATTR(csrfDenied, :Local :Private :AutoArgs :ActionClass(RenderView))
 *     vod csrfDenied(Context *c);
 * };
 * @endcode
 *
 * <H4>AJAX and CSRF protection</H4>
 *
 * If you are using ajax to submit form requests or if you use AJAX without a HTML form, you have to provide the
 * CSRF token too. If you are using the normal way by setting a cookie, you can read the CSRF token from that cookie.
 * If you use the session to store the token, you have to include the token somewhere into the DOM tree from where
 * you can read it. You can than add the extracted token to the POST data of every POST request or you can can set
 * a custom X-CSRFToken header to the value of the CSRF token. The latter method is often easier, because many
 * JavaScript frameworks provide hooks that allow headers to be set on every request.
 *
 *
 * <H3>How it works</H3>
 *
 * On every request, a secret token is set that is stored in a cookie or in the user session and has to be send
 * back to the application when performing actions on unsafe methods like POST. The token stored in the cookie or
 * in the session is salted with another random value. The same secret with a different salt has than to be sent
 * to the application either via a hidden form field or via a HTTP request header.
 *
 * To get the form field you can use the <CODE>{% c_csrf_token %}</CODE> tag in your Grantlee templates. If you
 * are not using Grantlee or if you do not use a form but AJAX, you can use CSRFProtection::getToken() to place the
 * token somewhere in your DOM tree so that you can read it with JavaScript.
 *
 * <H3 ID="limitations">Limitations</H3>
 *
 * Subdomains within a site will be able to set cookies on the client for the whole domain. By setting the cookie
 * and using a corresponding token, subdomains will be able to circumvent the CSRF protection. The only way to
 * avoid this is to ensure that subdomains are controlled by trusted users (or, are at least unable to set cookies).
 * Note that even without CSRF, there are other vulnerabilities, such as session fixation, that make giving
 * subdomains to untrusted parties a bad idea, and these vulnerabilities cannot easily be fixed with current browsers.
 *
 * <H3>Configuration file options</H3>
 *
 * There are some options you can set in your application configuration file in the @c Cutelyst_CSRFProtection_Plugin section.
 *
 * @par cookie_age
 * @parblock
 * Integer value, default: 31449600
 *
 * The age/expiration time of the cookie in seconds.
 *
 * The reason for setting a long-lived expiration time is to avoid problems in the case of a user closing a browser or
 * bookmarking a page and then loading that page from a browser cache. Without persistent cookies, the form submission
 * would fail in this case.
 *
 * Some browsers (specifically Internet Explorer) can disallow the use of persistent cookies or can have the indexes
 * to the cookie jar corrupted on disk, thereby causing CSRF protection checks to (sometimes intermittently) fail.
 * Change this setting to @c 0 to use session-based CSRF cookies, which keep the cookies in-memory instead of on persistent
 * storage.
 * @endparblock
 *
 * @par cookie_domain
 * @parblock
 * String value, default: empty
 *
 * The domain to be used when setting the CSRF cookie. This can be useful for easily allowing cross-subdomain requests to
 * be excluded from the normal cross site request forgery protection. It should be set to a string such as ".example.com"
 * to allow a POST request from a form on one subdomain to be accepted by a view served from another subdomain.
 *
 * Please note that the presence of this setting does not imply that the CSRF protection is safe from cross-subdomain
 * attacks by default - please see the <A HREF="#limitations">limitations section</A>.
 * @endparblock
 *
 * @par cookie_secure
 * @parblock
 * Boolean value, default: @c false
 *
 * Whether to use a secure cookie for the CSRF cookie. If this is set to @c true, the cookie will be marked as @a secure,
 * which means browsers may ensure that the cookie is only sent with an HTTPS connection.
 * @endparblock
 *
 * @par trusted_origins
 * @parblock
 * String list, default: empty
 *
 * A comma separated list of hosts which are trusted origins for unsafe requests (e.g. POST). For a secure unsafe request,
 * the CSRF protection requires that the request have a @a Referer header that matches the origin present in the @a Host
 * header. This prevents, for example, a POST request from @c subdomain.example.com from succeeding against @c api.example.com.
 * If you need cross-origin unsafe requests over HTTPS, continuing the example, add @c "subdomain.example.com" to this list.
 * The setting also supports subdomains, so you could add @c ".example.com", for example, to allow access from all subdomains
 * of @c example.com.
 * @endparblock
 *
 * @par log_failed_ip
 * @parblock
 * Boolean value, default: @c false
 *
 * If this is set to @c true, the log output for failed checks will contain the IP address of the remote client.
 * @endparblock
 *
 * <H3>Build options</H3>
 * This plugin is not enabled by default. Use <CODE>-DPLUGIN_CSRFPROTECTION:BOOL=ON</CODE> for your cmake configuration. To link it to your
 * application use @c %Cutelyst::CSRFProtection.
 *
 * @par Logging category
 * @c cutelyst.plugin.csrfprotection
 *
 * @since Cutelyst 1.12.0
 */
class CUTELYST_PLUGIN_CSRFPROTECTION_EXPORT CSRFProtection : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CSRFProtection)
public:
    /**
     * Constructs a new CSRFProtection object with the given @a parent.
     */
    CSRFProtection(Application *parent);

    /**
     * Deconstructs the CSRFProtection object.
     */
    virtual ~CSRFProtection();

    /**
     * Sets a default action the application will @link Context::detach() detach to @endlink if
     * the check for token and cookie failed. The default value is empty, so that there will be
     * no detaching and a generic error page will be generated.
     */
    void setDefaultDetachTo(const QString &actionNameOrPath);

    /**
     * Sets the name for the hidden form field that takes the CSRF token. This field name is used
     * by CSRFProtection::getTokenFormField(). The default value is @a "csrfprotectiontoken".
     */
    void setFormFieldName(const QString &fieldName);

    /**
     * Sets the name of the stash key that contains the error message if the CSRF protection check failed.
     */
    void setErrorMsgStashKey(const QString &keyName);

    /**
     * Sets a list of namespaces that should be completely exempted from CSRF protection. If you
     * have single methods in your namespaces that should still be protected, use the <CODE>:CSRFRequire</CODE>
     * attribute on this methods.
     */
    void setExemptedNamespaces(const QStringList &namespaces);

    /**
     * If this is set to @c true, the secret token will not be safed in a cookie but in the user's session.
     * For this, the Session plugin has to be available.
     *
     * Storing the token in a cookie (the default) is safe, but storing it in the session is common practice
     * in other web frameworks and therefore sometimes demaned by security auditors.
     */
    void setUseSessions(bool useSessions);

    /**
     * Whether to use HttpOnly flag on the CSRF cookie. If this is set to @c true, client-side JavaScript will
     * not to be able to access the CSRF cookie. The default is @c false.
     *
     * Designating the CSRF cookie as @c HttpOnly doesn’t offer any practical protection because CSRF is only
     * to protect against cross-domain attacks. If an attacker can read the cookie via JavaScript, they’re
     * already on the same domain as far as the browser knows, so they can do anything they like anyway.
     * (XSS is a much bigger hole than CSRF.)
     *
     * Although the setting offers little practical benefit, it’s sometimes required by security auditors.
     *
     * If you enable this and need to send the value of the CSRF token with an AJAX request, your JavaScript
     * must pull the value from a hidden CSRF token form input on the page instead of from the cookie.
     */
    void setCookieHttpOnly(bool httpOnly);

    /**
     * The name of the cookie to use for the CSRF authentication token. The default is @a "csrftoken". This can
     * be whatever you want (as long as it’s different from the other cookie names in your application).
     */
    void setCookieName(const QString &cookieName);

    /**
     * The name of the request header used for CSRF authentication. The header can contain the token if you don't
     * have a input form on your protected site. The default value is @a "X-CSRFTOKEN".
     */
    void setHeaderName(const QString &headerName);

    /**
     * Returns the current token.
     */
    static QByteArray getToken(Context *c);

    /**
     * Returns HTML code for a hidden input field that contains the current token and has the name
     * set by setFormInputName(). This method is also used by the Grantlee tag <CODE>{% c_csrf_token %}</CODE>
     *
     * @b Example output
     * @code{.html}
     * <input type="hidden" name="csrfprotectiontoken" value="e2RiYmI1YWJjLTJiZTctNDczYS1iMDM2ipApLPunLnnbkAQrzJWMo9GoyiQpzkeT">
     * @endcode
     */
    static QString getTokenFormField(Context *c);

protected:
    CSRFProtectionPrivate *d_ptr;

    virtual bool setup(Application *app) override;
};

}

#endif // CSRFPROTECTION_H
