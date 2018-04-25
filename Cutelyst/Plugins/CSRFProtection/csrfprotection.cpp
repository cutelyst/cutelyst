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

#include "csrfprotection_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Engine>
#include <Cutelyst/Context>
#include <Cutelyst/Request>
#include <Cutelyst/Response>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Headers>
#include <Cutelyst/Action>
#include <Cutelyst/Dispatcher>
#include <Cutelyst/Controller>

#include <QLoggingCategory>
#include <QNetworkCookie>
#include <QUuid>
#include <QUrl>
#include <vector>
#include <utility>
#include <algorithm>

#define DEFAULT_COOKIE_AGE Q_INT64_C(31449600) // approx. 1 year
#define DEFAULT_COOKIE_NAME "csrftoken"
#define DEFAULT_COOKIE_PATH "/"
#define DEFAULT_HEADER_NAME "X_CSRFTOKEN"
#define DEFAULT_FORM_INPUT_NAME "csrfprotectiontoken"
#define CSRF_SECRET_LENGTH 32
#define CSRF_TOKEN_LENGTH 2 * CSRF_SECRET_LENGTH
#define CSRF_ALLOWED_CHARS "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_"
#define CSRF_SESSION_KEY "_csrftoken"
#define CONTEXT_CSRF_COOKIE QStringLiteral("_c_csrfcookie")
#define CONTEXT_CSRF_COOKIE_USED QStringLiteral("_c_csrfcookieused")
#define CONTEXT_CSRF_COOKIE_NEEDS_RESET QStringLiteral("_c_csrfcookieneedsreset")
#define CONTEXT_CSRF_PROCESSING_DONE QStringLiteral("_c_csrfprocessingdone")
#define CONTEXT_CSRF_COOKIE_SET QStringLiteral("_c_csrfcookieset")
#define CONTEXT_CSRF_CHECK_PASSED QStringLiteral("_c_csrfcheckpassed")

Q_LOGGING_CATEGORY(C_CSRFPROTECTION, "cutelyst.plugin.csrfprotection")

using namespace Cutelyst;

static thread_local CSRFProtection *csrf = nullptr;
const QRegularExpression CSRFProtectionPrivate::sanitizeRe = QRegularExpression(QStringLiteral("[^a-zA-Z0-9\\-_]"));
// Assume that anything not defined as 'safe' by RFC7231 needs protection
const QStringList CSRFProtectionPrivate::secureMethods = QStringList({QStringLiteral("GET"), QStringLiteral("HEAD"), QStringLiteral("OPTIONS"), QStringLiteral("TRACE")});

CSRFProtection::CSRFProtection(Application *parent) : Plugin(parent)
  , d_ptr(new CSRFProtectionPrivate)
{

}

CSRFProtection::~CSRFProtection()
{
    delete d_ptr;
}

bool CSRFProtection::setup(Application *app)
{
    Q_D(CSRFProtection);

    app->loadTranslations(QStringLiteral("plugin_csrfprotection"));

    const QVariantMap config = app->engine()->config(QStringLiteral("Cutelyst_CSRFProtection_Plugin"));

    d->cookieAge = config.value(QStringLiteral("cookie_age"), DEFAULT_COOKIE_AGE).value<qint64>();
    if (d->cookieAge <= 0) {
        d->cookieAge = DEFAULT_COOKIE_AGE;
    }
    d->cookieDomain = config.value(QStringLiteral("cookie_domain")).toString();
    if (d->cookieName.isEmpty()) {
        d->cookieName = QStringLiteral(DEFAULT_COOKIE_NAME);
    }
    d->cookiePath = QStringLiteral(DEFAULT_COOKIE_PATH);
    d->cookieSecure = config.value(QStringLiteral("cookie_secure"), false).toBool();
    if (d->headerName.isEmpty()) {
        d->headerName = QStringLiteral(DEFAULT_HEADER_NAME);
    }
    d->trustedOrigins = config.value(QStringLiteral("trusted_origins")).toString().split(QLatin1Char(','), QString::SkipEmptyParts);
    if (d->formInputName.isEmpty()) {
        d->formInputName = QStringLiteral(DEFAULT_FORM_INPUT_NAME);
    }
    d->logFailedIp = config.value(QStringLiteral("log_failed_ip"), false).toBool();
    if (d->errorMsgStashKey.isEmpty()) {
        d->errorMsgStashKey = QStringLiteral("error_msg");
    }

    connect(app, &Application::postForked, this, [](Application *app){
        csrf = app->plugin<CSRFProtection *>();
    });

    connect(app, &Application::beforeDispatch, this, [d](Context *c) {
        d->beforeDispatch(c);
    });

    return true;
}

void CSRFProtection::setDefaultDetachTo(const QString &actionNameOrPath)
{
    Q_D(CSRFProtection);
    d->defaultDetachTo = actionNameOrPath;
}

void CSRFProtection::setFormFieldName(const QString &fieldName)
{
    Q_D(CSRFProtection);
    if (!fieldName.isEmpty()) {
        d->formInputName = fieldName;
    } else {
        d->formInputName = QStringLiteral(DEFAULT_FORM_INPUT_NAME);
    }
}

void CSRFProtection::setErrorMsgStashKey(const QString &keyName)
{
    Q_D(CSRFProtection);
    if (!keyName.isEmpty()) {
        d->errorMsgStashKey = keyName;
    } else {
        d->errorMsgStashKey = QStringLiteral("error_msg");
    }
}

void CSRFProtection::setIgnoredNamespaces(const QStringList &namespaces)
{
    Q_D(CSRFProtection);
    d->ignoredNamespaces = namespaces;
}

void CSRFProtection::setUseSessions(bool useSessions)
{
    Q_D(CSRFProtection);
    d->useSessions = useSessions;
}

void CSRFProtection::setCookieHttpOnly(bool httpOnly)
{
    Q_D(CSRFProtection);
    d->cookieHttpOnly = httpOnly;
}

void CSRFProtection::setCookieName(const QString &cookieName)
{
    Q_D(CSRFProtection);
    d->cookieName = cookieName;
}

void CSRFProtection::setHeaderName(const QString &headerName)
{
    Q_D(CSRFProtection);
    d->headerName = headerName;
}

void CSRFProtection::setGenericErrorMessage(const QString &message)
{
    Q_D(CSRFProtection);
    d->genericErrorMessage = message;
}

void CSRFProtection::setGenericErrorContentTyp(const QString &type)
{
    Q_D(CSRFProtection);
    d->genericContentType = type;
}

QByteArray CSRFProtection::getToken(Context *c)
{
    QByteArray token;

    const QByteArray contextCookie = c->stash(CONTEXT_CSRF_COOKIE).toByteArray();
    QByteArray secret;
    if (contextCookie.isEmpty()) {
        secret = CSRFProtectionPrivate::getNewCsrfString();
        token = CSRFProtectionPrivate::saltCipherSecret(secret);
        c->setStash(CONTEXT_CSRF_COOKIE, token);
    } else {
        secret = CSRFProtectionPrivate::unsaltCipherToken(contextCookie);
        token = CSRFProtectionPrivate::saltCipherSecret(secret);
    }

    c->setStash(CONTEXT_CSRF_COOKIE_USED, true);

    return token;
}

QString CSRFProtection::getTokenFormField(Context *c)
{
    QString form;

    if (!csrf) {
        qCCritical(C_CSRFPROTECTION) << "CSRFProtection plugin not registered";
        return form;
    }

    form = QStringLiteral("<input type=\"hidden\" name=\"%1\" value=\"%2\">").arg(csrf->d_ptr->formInputName, QString::fromLatin1(CSRFProtection::getToken(c)));

    return form;
}

bool CSRFProtection::checkPassed(Context *c)
{
    if (CSRFProtectionPrivate::secureMethods.contains(c->req()->method())) {
        return true;
    } else {
        return c->stash(CONTEXT_CSRF_CHECK_PASSED).toBool();
    }
}

//void CSRFProtection::rotateToken(Context *c)
//{
//    c->setStash(CONTEXT_CSRF_COOKIE_USED, true);
//    c->setStash(CONTEXT_CSRF_COOKIE, CSRFProtectionPrivate::getNewCsrfToken());
//    c->setStash(CONTEXT_CSRF_COOKIE_NEEDS_RESET, true);
//}

/**
 * @internal
 * Creates a new random string of length CSRF_SECRET_LENGTH by creating a uuid.
 */
QByteArray CSRFProtectionPrivate::getNewCsrfString()
{
    QByteArray csrfString;

    while (csrfString.size() < CSRF_SECRET_LENGTH) {
        csrfString.append(QUuid::createUuid().toRfc4122().toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    }

    csrfString.resize(CSRF_SECRET_LENGTH);

    return csrfString;
}

/**
 * @internal
 * Given a @a secret (assumed to be astring of CSRF_ALLOWED_CHARS), generate a
 * token by adding a salt and using it to encrypt the secret.
 */
QByteArray CSRFProtectionPrivate::saltCipherSecret(const QByteArray &secret)
{
    QByteArray salted;
    salted.reserve(CSRF_TOKEN_LENGTH);

    const QByteArray salt = CSRFProtectionPrivate::getNewCsrfString();
    const QByteArray chars = QByteArrayLiteral(CSRF_ALLOWED_CHARS);
    std::vector<std::pair<int,int>> pairs;
    pairs.reserve(std::min(secret.size(), salt.size()));
    for (int i = 0; i < std::min(secret.size(), salt.size()); ++i) {
        pairs.push_back(std::make_pair(chars.indexOf(secret.at(i)), chars.indexOf(salt.at(i))));
    }

    QByteArray cipher;
    cipher.reserve(CSRF_SECRET_LENGTH);
    for (std::size_t i = 0; i < pairs.size(); ++i) {
        const std::pair<int,int> p = pairs.at(i);
        cipher.append(chars[(p.first + p.second) % chars.size()]);
    }

    salted = salt + cipher;

    return salted;
}

/**
 * @internal
 * Given a @a token (assumed to be string fo CSRF_ALLOWED_CHARS, of length CSRF_TOKEN_LENGTH,
 * and that its first half is a salt), use it to decrypt the second half to produce the
 * original secret.
 */
QByteArray CSRFProtectionPrivate::unsaltCipherToken(const QByteArray &token)
{
    QByteArray secret;
    secret.reserve(CSRF_SECRET_LENGTH);

    const QByteArray salt = token.left(CSRF_SECRET_LENGTH);
    const QByteArray _token = token.mid(CSRF_SECRET_LENGTH);

    const QByteArray chars = QByteArrayLiteral(CSRF_ALLOWED_CHARS);
    std::vector<std::pair<int,int>> pairs;
    pairs.reserve(std::min(salt.size(), _token.size()));
    for (int i = 0; i < std::min(salt.size(), _token.size()); ++i) {
        pairs.push_back(std::make_pair(chars.indexOf(_token.at(i)), chars.indexOf(salt.at(i))));
    }


    for (std::size_t i = 0; i < pairs.size(); ++i) {
        const std::pair<int,int> p = pairs.at(i);
        int idx = p.first - p.second;
        if (idx < 0) {
            idx = chars.size() + idx;
        }
        secret.append(chars.at(idx));
    }

    return secret;
}

/**
 * @internal
 * Convenience function that creates a new token by creating a new random secret
 * and salting it.
 */
QByteArray CSRFProtectionPrivate::getNewCsrfToken()
{
    return CSRFProtectionPrivate::saltCipherSecret(CSRFProtectionPrivate::getNewCsrfString());
}

/**
 * @internal
 * Checks the @a token for conformance and creates a new token if it does not conform
 * to the allowed characters and/or length.
 */
QByteArray CSRFProtectionPrivate::sanitizeToken(const QByteArray &token)
{
    QByteArray sanitized;

    const QString tokenString = QString::fromLatin1(token);
    if (tokenString.contains(CSRFProtectionPrivate::sanitizeRe)) {
        sanitized = CSRFProtectionPrivate::getNewCsrfToken();
    } else if (token.size() != CSRF_TOKEN_LENGTH) {
        sanitized = CSRFProtectionPrivate::getNewCsrfToken();
    } else {
        sanitized = token;
    }

    return sanitized;
}

/**
 * @internal
 * Gets the token from either the user's session or from the token cookie.
 */
QByteArray CSRFProtectionPrivate::getToken(Context *c)
{
    QByteArray token;

    if (!csrf) {
        qCCritical(C_CSRFPROTECTION) << "CSRFProtection plugin not registered";
        return token;
    }

    if (csrf->d_ptr->useSessions) {
        token = Session::value(c, QStringLiteral(CSRF_SESSION_KEY)).toByteArray();
    } else {
        QByteArray cookieToken = c->req()->cookie(csrf->d_ptr->cookieName).toLatin1();

        if (cookieToken.isEmpty()) {
            return token;
        }

        token = CSRFProtectionPrivate::sanitizeToken(cookieToken);
        if (token != cookieToken) {
            c->setStash(CONTEXT_CSRF_COOKIE_NEEDS_RESET, true);
        }
    }

    qCDebug(C_CSRFPROTECTION, "Got token \"%s\" from %s.", token.constData(), csrf->d_ptr->useSessions ? "session" : "cookie");

    return token;
}

/**
 * @internal
 * Sets the token to either the user's session or to the token cookie.
 */
void CSRFProtectionPrivate::setToken(Context *c)
{
    if (!csrf) {
        qCCritical(C_CSRFPROTECTION) << "CSRFProtection plugin not registered";
        return;
    }

    if (csrf->d_ptr->useSessions) {
        Session::setValue(c, QStringLiteral(CSRF_SESSION_KEY), c->stash(CONTEXT_CSRF_COOKIE).toByteArray());
    } else {
        QNetworkCookie cookie(csrf->d_ptr->cookieName.toLatin1(), c->stash(CONTEXT_CSRF_COOKIE).toByteArray());
        if (!csrf->d_ptr->cookieDomain.isEmpty()) {
            cookie.setDomain(csrf->d_ptr->cookieDomain);
        }
        cookie.setExpirationDate(QDateTime::currentDateTime().addSecs(csrf->d_ptr->cookieAge));
        cookie.setHttpOnly(csrf->d_ptr->cookieHttpOnly);
        cookie.setPath(csrf->d_ptr->cookiePath);
        cookie.setSecure(csrf->d_ptr->cookieSecure);
        c->res()->setCookie(cookie);
        c->res()->headers().pushHeader(QStringLiteral("Vary"), QStringLiteral("Cookie"));
    }

    qCDebug(C_CSRFPROTECTION, "Set token \"%s\" to %s.", c->stash(CONTEXT_CSRF_COOKIE).toByteArray().constData(), csrf->d_ptr->useSessions ? "session" : "cookie");
}

/**
 * @internal
 * Rejects the request by either detaching to an action that handles the failed check
 * or by setting a generic error message to the response body.
 */
void CSRFProtectionPrivate::reject(Context *c, const QString &logReason, const QString &displayReason)
{
    c->setStash(CONTEXT_CSRF_CHECK_PASSED, false);

    if (!csrf) {
        qCCritical(C_CSRFPROTECTION) << "CSRFProtection plugin not registered";
        return;
    }

    qCWarning(C_CSRFPROTECTION, "Forbidden: (%s): /%s [%s]", qPrintable(logReason), qPrintable(c->req()->path()), csrf->d_ptr->logFailedIp ? qPrintable(c->req()->addressString()) : "IP logging disabled");

    c->res()->setStatus(Response::Forbidden);
    c->setStash(csrf->d_ptr->errorMsgStashKey, displayReason);

    QString detachToCsrf = c->action()->attribute(QStringLiteral("CSRFDetachTo"));
    if (detachToCsrf.isEmpty()) {
        detachToCsrf = csrf->d_ptr->defaultDetachTo;
    }

    Action *detachToAction = nullptr;

    if (!detachToCsrf.isEmpty()) {
        detachToAction = c->controller()->actionFor(detachToCsrf);
        if (!detachToAction) {
            detachToAction = c->dispatcher()->getActionByPath(detachToCsrf);
        }
        if (!detachToAction) {
            qCWarning(C_CSRFPROTECTION, "Can not find action for \"%s\" to detach to.", qPrintable(detachToCsrf));
        }
    }

    if (detachToAction) {
        c->detach(detachToAction);
    } else {
        if (!csrf->d_ptr->genericErrorMessage.isEmpty()) {
            c->res()->setBody(csrf->d_ptr->genericErrorMessage);
            c->res()->setContentType(csrf->d_ptr->genericContentType);
        } else {
            const QString title = c->translate("Cutelyst::CSRFProtection", "403 Forbidden - CSRF protection check failed");
            c->res()->setBody(QStringLiteral("<!DOCTYPE html><html><head><meta charset='utf-8'><title>") + title + QStringLiteral("</title></head><body><h1>") + title + QStringLiteral("</h1><p>") + displayReason + QStringLiteral("</p></body></html>"));
            c->res()->setContentType(QStringLiteral("text/html; charset=utf-8"));
        }
        c->detach();
    }
}

void CSRFProtectionPrivate::accept(Context *c)
{
    c->setStash(CONTEXT_CSRF_CHECK_PASSED, true);
    c->setStash(CONTEXT_CSRF_PROCESSING_DONE, true);
}

/**
 * @internal
 * Compares two salted tokens that are assumed to be both sanitized.
 */
bool CSRFProtectionPrivate::compareSaltedTokens(const QByteArray &t1, const QByteArray &t2)
{
    const QByteArray _t1 = CSRFProtectionPrivate::unsaltCipherToken(t1);
    const QByteArray _t2 = CSRFProtectionPrivate::unsaltCipherToken(t2);

    // to avoid timing attack
    int diff = _t1.size() ^ _t2.size();
    for (int i = 0; i < _t1.size() && i < _t2.size(); i++) {
        diff |= _t1[i]  ^ _t2[i];
    }
    return diff == 0;
}

/**
 * @internal
 * Sets and checks the tokens before the target action has been dispatched.
 */
void CSRFProtectionPrivate::beforeDispatch(Context *c)
{
    if (!csrf) {
        CSRFProtectionPrivate::reject(c, QStringLiteral("CSRFProtection plugin not registered"), c->translate("Cutelyst::CSRFProtection", "The CSRF protection plugin has not been registered."));
        return;
    }

    const QByteArray csrfToken = CSRFProtectionPrivate::getToken(c);
    if (!csrfToken.isNull()) {
        c->setStash(CONTEXT_CSRF_COOKIE, csrfToken);
    } else {
        CSRFProtection::getToken(c);
    }

    if (c->stash(CONTEXT_CSRF_PROCESSING_DONE).toBool()) {
        return;
    }

    if (c->action()->attributes().contains(QStringLiteral("CSRFIgnore"))) {
        qCDebug(C_CSRFPROTECTION, "Action \"%s::%s\" is ignored by the CSRF protection.", qPrintable(c->action()->className()), qPrintable(c->action()->reverse()));
        return;
    }

    if (csrf->d_ptr->ignoredNamespaces.contains(c->action()->ns())) {
        if (!c->action()->attributes().contains(QStringLiteral("CSRFRequire"))) {
            qCDebug(C_CSRFPROTECTION, "Namespace \"%s\" is ignored by the CSRF protection.", qPrintable(c->action()->ns()));
            return;
        }
    }

    // only check the tokens if the method is not secure, e.g. POST
    // the following methods are secure according to RFC 7231: GET, HEAD, OPTIONS and TRACE
    if (!CSRFProtectionPrivate::secureMethods.contains(c->req()->method())) {

        bool ok = true;

        // Suppose user visits http://example.com/
        // An active network attacker (man-in-the-middle, MITM) sends a POST form that targets
        // https://example.com/detonate-bomb/ and submits it via JavaScript.
        //
        // The attacker will need to provide a CSRF cookie and token, but that's no problem for a
        // MITM and the session-independent secret we're using. So the MITM can circumvent the CSRF
        // protection. This is true for any HTTP connection, but anyone using HTTPS expects better!
        // For this reason, for https://example.com/ we need additional protection that treats
        // http://example.com/ as completely untrusted. Under HTTPS, Barth et al. found that the
        // Referer header is missing for same-domain requests in only about 0.2% of cases or less, so
        // we can use strict Referer checking.
        if (c->req()->secure()) {
            const QString referer = c->req()->headers().referer();

            if (Q_UNLIKELY(referer.isEmpty())) {
                CSRFProtectionPrivate::reject(c, QStringLiteral("Referer checking failed - no Referer."), c->translate("Cutelyst::CSRFProtection", "Referer checking failed - no Referer."));
                ok = false;
            } else {
                const QUrl refererUrl(referer);
                if (Q_UNLIKELY(!refererUrl.isValid())) {
                    CSRFProtectionPrivate::reject(c, QStringLiteral("Referer checking failed - Referer is malformed."), c->translate("Cutelyst::CSRFProtection", "Referer checking failed - Referer is malformed."));
                    ok = false;
                } else {
                    if (Q_UNLIKELY(refererUrl.scheme() != QLatin1String("https"))) {
                        CSRFProtectionPrivate::reject(c, QStringLiteral("Referer checking failed - Referer is insecure while host is secure."), c->translate("Cutelyst::CSRFProtection", "Referer checking failed - Referer is insecure while host is secure."));
                        ok = false;
                    } else {
                        // If there isn't a CSRF_COOKIE_DOMAIN, require an exact match on host:port.
                        // If not, obey the cookie rules (or those for the session cookie, if we
                        // use sessions
                        const QUrl uri = c->req()->uri();
                        QString goodReferer;
                        if (!csrf->d_ptr->useSessions) {
                            goodReferer = csrf->d_ptr->cookieDomain;
                        }
                        if (goodReferer.isEmpty()) {
                            goodReferer = uri.host();
                        }
                        const int serverPort = uri.port(c->req()->secure() ? 443 : 80);
                        if ((serverPort != 80) && (serverPort != 443)) {
                            goodReferer += QLatin1Char(':') + QString::number(serverPort);
                        }

                        QStringList goodHosts = csrf->d_ptr->trustedOrigins;
                        goodHosts.append(goodReferer);

                        QString refererHost = refererUrl.host();
                        const int refererPort = refererUrl.port(refererUrl.scheme() == QLatin1String("https") ? 443 : 80);
                        if ((refererPort != 80) && (refererPort != 443)) {
                            refererHost += QLatin1Char(':') + QString::number(refererPort);
                        }

                        bool refererCheck = false;
                        for (int i = 0; i < goodHosts.size(); ++i) {
                            const QString host = goodHosts.at(i);
                            if ((host.startsWith(QLatin1Char('.')) && (refererHost.endsWith(host) || (refererHost == host.mid(1)))) || host == refererHost) {
                                refererCheck = true;
                                break;
                            }
                        }

                        if (Q_UNLIKELY(!refererCheck)) {
                            ok = false;
                            CSRFProtectionPrivate::reject(c, QStringLiteral("Referer checking failed - %1 does not match any trusted origins.").arg(referer), c->translate("Cutelyst::CSRFProtection", "Referer checking failed - %1 does not match any trusted origins.").arg(referer));
                        }
                    }
                }
            }
        }

        if (Q_LIKELY(ok)) {
            if (Q_UNLIKELY(csrfToken.isEmpty())) {
                CSRFProtectionPrivate::reject(c, QStringLiteral("CSRF cookie not set."), c->translate("Cutelyst::CSRFProtection", "CSRF cookie not set."));
                ok = false;
            } else {

                QByteArray requestCsrfToken;
                // delete does not have body data
                if (c->req()->method() != QLatin1String("DELETE")) {
                    requestCsrfToken = c->req()->bodyParam(csrf->d_ptr->formInputName).toLatin1();
                }

                if (requestCsrfToken.isEmpty()) {
                    requestCsrfToken = c->req()->header(csrf->d_ptr->headerName).toLatin1();
                    if (Q_LIKELY(!requestCsrfToken.isEmpty())) {
                        qCDebug(C_CSRFPROTECTION, "Got token \"%s\" from HTTP header %s.", requestCsrfToken.constData(), qPrintable(csrf->d_ptr->headerName));
                    } else {
                        qCDebug(C_CSRFPROTECTION, "Can not get token from HTTP header or form field.");
                    }
                } else {
                    qCDebug(C_CSRFPROTECTION, "Got token \"%s\" from form field %s.", requestCsrfToken.constData(), qPrintable(csrf->d_ptr->formInputName));
                }

                requestCsrfToken = CSRFProtectionPrivate::sanitizeToken(requestCsrfToken);

                if (Q_UNLIKELY(!CSRFProtectionPrivate::compareSaltedTokens(requestCsrfToken, csrfToken))) {
                    CSRFProtectionPrivate::reject(c, QStringLiteral("CSRF token missing or incorrect."), c->translate("Cutelyst::CSRFProtection", "CSRF token missing or incorrect."));
                    ok = false;
                }
            }
        }

        if (Q_LIKELY(ok)) {
            CSRFProtectionPrivate::accept(c);
        }
    }

    // Set the CSRF cookie even if it's already set, so we renew
    // the expiry timer.

    if (!c->stash(CONTEXT_CSRF_COOKIE_NEEDS_RESET).toBool()) {
        if (c->stash(CONTEXT_CSRF_COOKIE_SET).toBool()) {
            return;
        }
    }

    if (!c->stash(CONTEXT_CSRF_COOKIE_USED).toBool()) {
        return;
    }

    CSRFProtectionPrivate::setToken(c);
    c->setStash(CONTEXT_CSRF_COOKIE_SET, true);
}

#include "moc_csrfprotection.cpp"
