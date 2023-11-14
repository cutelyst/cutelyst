/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "csrfprotection_p.h"

#include <Cutelyst/Action>
#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Controller>
#include <Cutelyst/Dispatcher>
#include <Cutelyst/Engine>
#include <Cutelyst/Headers>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Request>
#include <Cutelyst/Response>
#include <Cutelyst/Upload>
#include <Cutelyst/utils.h>
#include <algorithm>
#include <utility>
#include <vector>

#include <QLoggingCategory>
#include <QNetworkCookie>
#include <QUrl>
#include <QUuid>

Q_LOGGING_CATEGORY(C_CSRFPROTECTION, "cutelyst.plugin.csrfprotection", QtWarningMsg)

using namespace Cutelyst;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static thread_local CSRFProtection *csrf = nullptr;
const QRegularExpression CSRFProtectionPrivate::sanitizeRe{u"[^a-zA-Z0-9\\-_]"_qs};
// Assume that anything not defined as 'safe' by RFC7231 needs protection
const QByteArrayList CSRFProtectionPrivate::secureMethods = QByteArrayList({
    "GET",
    "HEAD",
    "OPTIONS",
    "TRACE",
});
const QByteArray CSRFProtectionPrivate::allowedChars{
    "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-_"_qba};
const QString CSRFProtectionPrivate::sessionKey{u"_csrftoken"_qs};
const QString CSRFProtectionPrivate::stashKeyCookie{u"_c_csrfcookie"_qs};
const QString CSRFProtectionPrivate::stashKeyCookieUsed{u"_c_csrfcookieused"_qs};
const QString CSRFProtectionPrivate::stashKeyCookieNeedsReset{u"_c_csrfcookieneedsreset"_qs};
const QString CSRFProtectionPrivate::stashKeyCookieSet{u"_c_csrfcookieset"_qs};
const QString CSRFProtectionPrivate::stashKeyProcessingDone{u"_c_csrfprocessingdone"_qs};
const QString CSRFProtectionPrivate::stashKeyCheckPassed{u"_c_csrfcheckpassed"_qs};

CSRFProtection::CSRFProtection(Application *parent)
    : Plugin(parent)
    , d_ptr(new CSRFProtectionPrivate)
{
}

CSRFProtection::~CSRFProtection() = default;

bool CSRFProtection::setup(Application *app)
{
    Q_D(CSRFProtection);

    app->loadTranslations(u"plugin_csrfprotection"_qs);

    const QVariantMap config = app->engine()->config(u"Cutelyst_CSRFProtection_Plugin"_qs);

    bool cookieExpirationOk = false;
    const QString cookieExpireStr =
        config
            .value(
                u"cookie_expiration"_qs,
                config.value(u"cookie_age"_qs,
                             static_cast<qint64>(std::chrono::duration_cast<std::chrono::seconds>(
                                                     CSRFProtectionPrivate::cookieDefaultExpiration)
                                                     .count())))
            .toString();
    d->cookieExpiration = std::chrono::duration_cast<std::chrono::seconds>(
        Utils::durationFromString(cookieExpireStr, &cookieExpirationOk));
    if (!cookieExpirationOk) {
        qCWarning(C_CSRFPROTECTION).nospace() << "Invalid value set for cookie_expiration. "
                                                 "Using default value "
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
                                              << CSRFProtectionPrivate::cookieDefaultExpiration;
#else
                                              << "1 year";
#endif
        d->cookieExpiration = CSRFProtectionPrivate::cookieDefaultExpiration;
    }

    d->cookieDomain = config.value(u"cookie_domain"_qs).toString();
    if (d->cookieName.isEmpty()) {
        d->cookieName = "csrftoken";
    }
    d->cookiePath = u"/"_qs;

    const QString _sameSite = config.value(u"cookie_same_site"_qs, u"strict"_qs).toString();
    if (_sameSite.compare(u"default", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Default;
    } else if (_sameSite.compare(u"none", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::None;
    } else if (_sameSite.compare(u"lax", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Lax;
    } else if (_sameSite.compare(u"strict", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Strict;
    } else {
        qCWarning(C_CSRFPROTECTION).nospace() << "Invalid value set for cookie_same_site. "
                                                 "Using default value "
                                              << QNetworkCookie::SameSite::Strict;
        d->cookieSameSite = QNetworkCookie::SameSite::Strict;
    }

    d->cookieSecure = config.value(u"cookie_secure"_qs, false).toBool();

    if ((d->cookieSameSite == QNetworkCookie::SameSite::None) && !d->cookieSecure) {
        qCWarning(C_CSRFPROTECTION)
            << "cookie_same_site has been set to None but cookie_secure is "
               "not set to true. Implicitely setting cookie_secure to true. "
               "Please check your configuration.";
        d->cookieSecure = true;
    }

    if (d->headerName.isEmpty()) {
        d->headerName = "X_CSRFTOKEN";
    }

    d->trustedOrigins =
        config.value(u"trusted_origins"_qs).toString().split(u',', Qt::SkipEmptyParts);
    if (d->formInputName.isEmpty()) {
        d->formInputName = "csrfprotectiontoken";
    }
    d->logFailedIp = config.value(u"log_failed_ip"_qs, false).toBool();
    if (d->errorMsgStashKey.isEmpty()) {
        d->errorMsgStashKey = u"error_msg"_qs;
    }

    connect(app, &Application::postForked, this, [](Application *app) {
        csrf = app->plugin<CSRFProtection *>();
    });

    connect(app, &Application::beforeDispatch, this, [d](Context *c) { d->beforeDispatch(c); });

    return true;
}

void CSRFProtection::setDefaultDetachTo(const QString &actionNameOrPath)
{
    Q_D(CSRFProtection);
    d->defaultDetachTo = actionNameOrPath;
}

void CSRFProtection::setFormFieldName(const QByteArray &fieldName)
{
    Q_D(CSRFProtection);
    d->formInputName = fieldName;
}

void CSRFProtection::setErrorMsgStashKey(const QString &keyName)
{
    Q_D(CSRFProtection);
    d->errorMsgStashKey = keyName;
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

void CSRFProtection::setCookieName(const QByteArray &cookieName)
{
    Q_D(CSRFProtection);
    d->cookieName = cookieName;
}

void CSRFProtection::setHeaderName(const QByteArray &headerName)
{
    Q_D(CSRFProtection);
    d->headerName = headerName;
}

void CSRFProtection::setGenericErrorMessage(const QString &message)
{
    Q_D(CSRFProtection);
    d->genericErrorMessage = message;
}

void CSRFProtection::setGenericErrorContentType(const QByteArray &type)
{
    Q_D(CSRFProtection);
    d->genericContentType = type;
}

QByteArray CSRFProtection::getToken(Context *c)
{
    QByteArray token;

    const QByteArray contextCookie = c->stash(CSRFProtectionPrivate::stashKeyCookie).toByteArray();
    QByteArray secret;
    if (contextCookie.isEmpty()) {
        secret = CSRFProtectionPrivate::getNewCsrfString();
        token  = CSRFProtectionPrivate::saltCipherSecret(secret);
        c->setStash(CSRFProtectionPrivate::stashKeyCookie, token);
    } else {
        secret = CSRFProtectionPrivate::unsaltCipherToken(contextCookie);
        token  = CSRFProtectionPrivate::saltCipherSecret(secret);
    }

    c->setStash(CSRFProtectionPrivate::stashKeyCookieUsed, true);

    return token;
}

QString CSRFProtection::getTokenFormField(Context *c)
{
    QString form;

    if (!csrf) {
        qCCritical(C_CSRFPROTECTION) << "CSRFProtection plugin not registered";
        return form;
    }

    form = QStringLiteral("<input type=\"hidden\" name=\"%1\" value=\"%2\" />")
               .arg(QString::fromLatin1(csrf->d_ptr->formInputName),
                    QString::fromLatin1(CSRFProtection::getToken(c)));

    return form;
}

bool CSRFProtection::checkPassed(Context *c)
{
    if (CSRFProtectionPrivate::secureMethods.contains(c->req()->method())) {
        return true;
    } else {
        return c->stash(CSRFProtectionPrivate::stashKeyCheckPassed).toBool();
    }
}

// void CSRFProtection::rotateToken(Context *c)
//{
//     c->setStash(CSRFProtectionPrivate::stashKeyCookieUsed, true);
//     c->setStash(QString CSRFProtectionPrivate::stashKeyCookie,
//     CSRFProtectionPrivate::getNewCsrfToken());
//     c->setStash(CSRFProtectionPrivate::stashKeyCookieNeedsReset, true);
// }

/**
 * @internal
 * Creates a new random string of length CSRFProtectionPrivate::secretLength by creating a uuid.
 */
QByteArray CSRFProtectionPrivate::getNewCsrfString()
{
    QByteArray csrfString;

    while (csrfString.size() < CSRFProtectionPrivate::secretLength) {
        csrfString.append(QUuid::createUuid().toRfc4122().toBase64(QByteArray::Base64UrlEncoding |
                                                                   QByteArray::OmitTrailingEquals));
    }

    csrfString.resize(CSRFProtectionPrivate::secretLength);

    return csrfString;
}

/**
 * @internal
 * Given a @a secret (assumed to be astring of CSRFProtectionPrivate::allowedChars), generate a
 * token by adding a salt and using it to encrypt the secret.
 */
QByteArray CSRFProtectionPrivate::saltCipherSecret(const QByteArray &secret)
{
    QByteArray salted;
    salted.reserve(CSRFProtectionPrivate::tokenLength);

    const QByteArray salt = CSRFProtectionPrivate::getNewCsrfString();
    std::vector<std::pair<int, int>> pairs;
    pairs.reserve(std::min(secret.size(), salt.size()));
    for (int i = 0; i < std::min(secret.size(), salt.size()); ++i) {
        pairs.emplace_back(CSRFProtectionPrivate::allowedChars.indexOf(secret.at(i)),
                           CSRFProtectionPrivate::allowedChars.indexOf(salt.at(i)));
    }

    QByteArray cipher;
    cipher.reserve(CSRFProtectionPrivate::secretLength);
    for (const auto &p : std::as_const(pairs)) {
        cipher.append(
            CSRFProtectionPrivate::allowedChars[(p.first + p.second) %
                                                CSRFProtectionPrivate::allowedChars.size()]);
    }

    salted = salt + cipher;

    return salted;
}

/**
 * @internal
 * Given a @a token (assumed to be string of CSRFProtectionPrivate::allowedChars, of length
 * CSRFProtectionPrivate::tokenLength, and that its first half is a salt), use it to decrypt the
 * second half to produce the original secret.
 */
QByteArray CSRFProtectionPrivate::unsaltCipherToken(const QByteArray &token)
{
    QByteArray secret;
    secret.reserve(CSRFProtectionPrivate::secretLength);

    const QByteArray salt   = token.left(CSRFProtectionPrivate::secretLength);
    const QByteArray _token = token.mid(CSRFProtectionPrivate::secretLength);

    std::vector<std::pair<int, int>> pairs;
    pairs.reserve(std::min(salt.size(), _token.size()));
    for (int i = 0; i < std::min(salt.size(), _token.size()); ++i) {
        pairs.emplace_back(CSRFProtectionPrivate::allowedChars.indexOf(_token.at(i)),
                           CSRFProtectionPrivate::allowedChars.indexOf(salt.at(i)));
    }

    for (const auto &p : std::as_const(pairs)) {
        QByteArray::size_type idx = p.first - p.second;
        if (idx < 0) {
            idx = CSRFProtectionPrivate::allowedChars.size() + idx;
        }
        secret.append(CSRFProtectionPrivate::allowedChars.at(idx));
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
    if (tokenString.contains(CSRFProtectionPrivate::sanitizeRe) ||
        token.size() != CSRFProtectionPrivate::tokenLength) {
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
        token = Session::value(c, CSRFProtectionPrivate::sessionKey).toByteArray();
    } else {
        QByteArray cookieToken = c->req()->cookie(csrf->d_ptr->cookieName);
        if (cookieToken.isEmpty()) {
            return token;
        }

        token = CSRFProtectionPrivate::sanitizeToken(cookieToken);
        if (token != cookieToken) {
            c->setStash(CSRFProtectionPrivate::stashKeyCookieNeedsReset, true);
        }
    }

    qCDebug(C_CSRFPROTECTION) << "Got token" << token << "from"
                              << (csrf->d_ptr->useSessions ? "sessions" : "cookie");

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
        Session::setValue(c,
                          CSRFProtectionPrivate::sessionKey,
                          c->stash(CSRFProtectionPrivate::stashKeyCookie).toByteArray());
    } else {
        QNetworkCookie cookie(csrf->d_ptr->cookieName,
                              c->stash(CSRFProtectionPrivate::stashKeyCookie).toByteArray());
        if (!csrf->d_ptr->cookieDomain.isEmpty()) {
            cookie.setDomain(csrf->d_ptr->cookieDomain);
        }
        if (csrf->d_ptr->cookieExpiration.count() == 0) {
            cookie.setExpirationDate(QDateTime());
        } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
            cookie.setExpirationDate(
                QDateTime::currentDateTime().addDuration(csrf->d_ptr->cookieExpiration));
#else
            cookie.setExpirationDate(
                QDateTime::currentDateTime().addSecs(csrf->d_ptr->cookieExpiration.count()));
#endif
        }
        cookie.setHttpOnly(csrf->d_ptr->cookieHttpOnly);
        cookie.setPath(csrf->d_ptr->cookiePath);
        cookie.setSecure(csrf->d_ptr->cookieSecure);
        cookie.setSameSitePolicy(csrf->d_ptr->cookieSameSite);
        c->res()->setCookie(cookie);
        c->res()->headers().pushHeader("Vary"_qba, "Cookie"_qba);
    }

    qCDebug(C_CSRFPROTECTION) << "Set token"
                              << c->stash(CSRFProtectionPrivate::stashKeyCookie).toByteArray()
                              << "to" << (csrf->d_ptr->useSessions ? "session" : "cookie");
}

/**
 * @internal
 * Rejects the request by either detaching to an action that handles the failed check
 * or by setting a generic error message to the response body.
 */
void CSRFProtectionPrivate::reject(Context *c,
                                   const QString &logReason,
                                   const QString &displayReason)
{
    c->setStash(CSRFProtectionPrivate::stashKeyCheckPassed, false);

    if (!csrf) {
        qCCritical(C_CSRFPROTECTION) << "CSRFProtection plugin not registered";
        return;
    }

    if (C_CSRFPROTECTION().isWarningEnabled()) {
        if (csrf->d_ptr->logFailedIp) {
            qCWarning(C_CSRFPROTECTION).nospace().noquote()
                << "Forbidden: (" << logReason << "): " << c->req()->path() << " ["
                << c->req()->addressString() << "]";
        } else {
            qCWarning(C_CSRFPROTECTION).nospace().noquote()
                << "Forbidden: (" << logReason << "): " << c->req()->path()
                << " [IP logging disabled]";
        }
    }

    c->res()->setStatus(Response::Forbidden);
    c->setStash(csrf->d_ptr->errorMsgStashKey, displayReason);

    QString detachToCsrf = c->action()->attribute(u"CSRFDetachTo"_qs);
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
            qCWarning(C_CSRFPROTECTION)
                << "Can not find action for" << detachToCsrf << "to detach to";
        }
    }

    if (detachToAction) {
        c->detach(detachToAction);
    } else {
        c->res()->setStatus(Response::Forbidden);
        if (!csrf->d_ptr->genericErrorMessage.isEmpty()) {
            c->res()->setBody(csrf->d_ptr->genericErrorMessage);
            c->res()->setContentType(csrf->d_ptr->genericContentType);
        } else {
            //% "403 Forbidden - CSRF protection check failed"
            const QString title = c->qtTrId("cutelyst-csrf-generic-error-page-title");
            c->res()->setBody(QStringLiteral("<!DOCTYPE html>\n"
                                             "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                                             "  <head>\n"
                                             "    <title>") +
                              title +
                              QStringLiteral("</title>\n"
                                             "  </head>\n"
                                             "  <body>\n"
                                             "    <h1>") +
                              title +
                              QStringLiteral("</h1>\n"
                                             "    <p>") +
                              displayReason +
                              QStringLiteral("</p>\n"
                                             "  </body>\n"
                                             "</html>\n"));
            c->res()->setContentType("text/html; charset=utf-8"_qba);
        }
        c->detach();
    }
}

void CSRFProtectionPrivate::accept(Context *c)
{
    c->setStash(CSRFProtectionPrivate::stashKeyCheckPassed, true);
    c->setStash(CSRFProtectionPrivate::stashKeyProcessingDone, true);
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
    QByteArray::size_type diff = _t1.size() ^ _t2.size();
    for (QByteArray::size_type i = 0; i < _t1.size() && i < _t2.size(); i++) {
        diff |= _t1[i] ^ _t2[i];
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
        CSRFProtectionPrivate::reject(c,
                                      u"CSRFProtection plugin not registered"_qs,
                                      //% "The CSRF protection plugin has not been registered."
                                      c->qtTrId("cutelyst-csrf-reject-not-registered"));
        return;
    }

    const QByteArray csrfToken = CSRFProtectionPrivate::getToken(c);
    if (!csrfToken.isNull()) {
        c->setStash(CSRFProtectionPrivate::stashKeyCookie, csrfToken);
    } else {
        CSRFProtection::getToken(c);
    }

    if (c->stash(CSRFProtectionPrivate::stashKeyProcessingDone).toBool()) {
        return;
    }

    if (c->action()->attributes().contains(u"CSRFIgnore"_qs)) {
        qCDebug(C_CSRFPROTECTION).noquote().nospace()
            << "Action " << c->action()->className() << "::" << c->action()->reverse()
            << " is ignored by the CSRF protection";
        return;
    }

    if (csrf->d_ptr->ignoredNamespaces.contains(c->action()->ns())) {
        if (!c->action()->attributes().contains(u"CSRFRequire"_qs)) {
            qCDebug(C_CSRFPROTECTION)
                << "Namespace" << c->action()->ns() << "is ignored by the CSRF protection";
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
        // Referer header is missing for same-domain requests in only about 0.2% of cases or less,
        // so we can use strict Referer checking.
        if (c->req()->secure()) {
            const auto referer = c->req()->headers().referer();

            if (Q_UNLIKELY(referer.isEmpty())) {
                CSRFProtectionPrivate::reject(c,
                                              u"Referer checking failed - no Referer"_qs,
                                              //% "Referrer checking failed - no Referrer."
                                              c->qtTrId("cutelyst-csrf-reject-no-referer"));
                ok = false;
            } else {
                const QUrl refererUrl(QString::fromLatin1(referer));
                if (Q_UNLIKELY(!refererUrl.isValid())) {
                    CSRFProtectionPrivate::reject(
                        c,
                        u"Referer checking failed - Referer is malformed"_qs,
                        //% "Referrer checking failed - Referrer is malformed."
                        c->qtTrId("cutelyst-csrf-reject-referer-malformed"));
                    ok = false;
                } else {
                    if (Q_UNLIKELY(refererUrl.scheme() != QLatin1String("https"))) {
                        CSRFProtectionPrivate::reject(
                            c,
                            u"Referer checking failed - Referer is insecure while "
                            "host is secure"_qs,
                            //% "Referrer checking failed - Referrer is insecure while host "
                            //% "is secure."
                            c->qtTrId("cutelyst-csrf-reject-refer-insecure"));
                        ok = false;
                    } else {
                        // If there isn't a CSRF_COOKIE_DOMAIN, require an exact match on host:port.
                        // If not, obey the cookie rules (or those for the session cookie, if we
                        // use sessions
                        constexpr int httpPort  = 80;
                        constexpr int httpsPort = 443;

                        const QUrl uri = c->req()->uri();
                        QString goodReferer;
                        if (!csrf->d_ptr->useSessions) {
                            goodReferer = csrf->d_ptr->cookieDomain;
                        }
                        if (goodReferer.isEmpty()) {
                            goodReferer = uri.host();
                        }
                        const int serverPort = uri.port(c->req()->secure() ? httpsPort : httpPort);
                        if ((serverPort != httpPort) && (serverPort != httpsPort)) {
                            goodReferer += u':' + QString::number(serverPort);
                        }

                        QStringList goodHosts = csrf->d_ptr->trustedOrigins;
                        goodHosts.append(goodReferer);

                        QString refererHost   = refererUrl.host();
                        const int refererPort = refererUrl.port(
                            refererUrl.scheme().compare(u"https") == 0 ? httpsPort : httpPort);
                        if ((refererPort != httpPort) && (refererPort != httpsPort)) {
                            refererHost += u':' + QString::number(refererPort);
                        }

                        bool refererCheck = false;
                        for (const auto &host : std::as_const(goodHosts)) {
                            if ((host.startsWith(u'.') &&
                                 (refererHost.endsWith(host) || (refererHost == host.mid(1)))) ||
                                host == refererHost) {
                                refererCheck = true;
                                break;
                            }
                        }

                        if (Q_UNLIKELY(!refererCheck)) {
                            ok = false;
                            CSRFProtectionPrivate::reject(
                                c,
                                u"Referer checking failed - %1 does not match any "
                                "trusted origins"_qs.arg(QString::fromLatin1(referer)),
                                //% "Referrer checking failed - %1 does not match any "
                                //% "trusted origin."
                                c->qtTrId("cutelyst-csrf-reject-referer-no-trust")
                                    .arg(QString::fromLatin1(referer)));
                        }
                    }
                }
            }
        }

        if (Q_LIKELY(ok)) {
            if (Q_UNLIKELY(csrfToken.isEmpty())) {
                CSRFProtectionPrivate::reject(c,
                                              u"CSRF cookie not set"_qs,
                                              //% "CSRF cookie not set."
                                              c->qtTrId("cutelyst-csrf-reject-no-cookie"));
                ok = false;
            } else {

                QByteArray requestCsrfToken;
                // delete does not have body data
                if (!c->req()->isDelete()) {
                    if (c->req()->contentType().compare("multipart/form-data") == 0) {
                        // everything is an upload, even our token
                        Upload *upload =
                            c->req()->upload(QString::fromLatin1(csrf->d_ptr->formInputName));
                        if (upload && upload->size() < 1024 /*FIXME*/) {
                            requestCsrfToken = upload->readAll();
                        }
                    } else
                        requestCsrfToken =
                            c->req()
                                ->bodyParam(QString::fromLatin1(csrf->d_ptr->formInputName))
                                .toLatin1();
                }

                if (requestCsrfToken.isEmpty()) {
                    requestCsrfToken = c->req()->header(csrf->d_ptr->headerName);
                    if (Q_LIKELY(!requestCsrfToken.isEmpty())) {
                        qCDebug(C_CSRFPROTECTION) << "Got token" << requestCsrfToken
                                                  << "from HTTP header" << csrf->d_ptr->headerName;
                    } else {
                        qCDebug(C_CSRFPROTECTION)
                            << "Can not get token from HTTP header or form field.";
                    }
                } else {
                    qCDebug(C_CSRFPROTECTION) << "Got token" << requestCsrfToken
                                              << "from form field" << csrf->d_ptr->formInputName;
                }

                requestCsrfToken = CSRFProtectionPrivate::sanitizeToken(requestCsrfToken);

                if (Q_UNLIKELY(
                        !CSRFProtectionPrivate::compareSaltedTokens(requestCsrfToken, csrfToken))) {
                    CSRFProtectionPrivate::reject(c,
                                                  u"CSRF token missing or incorrect"_qs,
                                                  //% "CSRF token missing or incorrect."
                                                  c->qtTrId("cutelyst-csrf-reject-token-missin"));
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

    if (!c->stash(CSRFProtectionPrivate::stashKeyCookieNeedsReset).toBool()) {
        if (c->stash(CSRFProtectionPrivate::stashKeyCookieSet).toBool()) {
            return;
        }
    }

    if (!c->stash(CSRFProtectionPrivate::stashKeyCookieUsed).toBool()) {
        return;
    }

    CSRFProtectionPrivate::setToken(c);
    c->setStash(CSRFProtectionPrivate::stashKeyCookieSet, true);
}

#include "moc_csrfprotection.cpp"
