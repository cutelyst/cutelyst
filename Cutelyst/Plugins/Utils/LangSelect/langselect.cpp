/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "langselect_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Engine>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Response>
#include <Cutelyst/utils.h>
#include <map>
#include <utility>

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QUrl>
#include <QUrlQuery>

#if defined(QT_DEBUG)
Q_LOGGING_CATEGORY(C_LANGSELECT, "cutelyst.plugin.langselect")
#else
Q_LOGGING_CATEGORY(C_LANGSELECT, "cutelyst.plugin.langselect", QtWarningMsg)
#endif

using namespace Cutelyst;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static thread_local LangSelect *lsp = nullptr;

const QString LangSelectPrivate::stashKeySelectionTried{u"_c_langselect_tried"_qs};

LangSelect::LangSelect(Application *parent, Cutelyst::LangSelect::Source source)
    : Plugin(parent)
    , d_ptr(new LangSelectPrivate)
{
    Q_D(LangSelect);
    d->source     = source;
    d->autoDetect = true;
}

LangSelect::LangSelect(Application *parent)
    : Plugin(parent)
    , d_ptr(new LangSelectPrivate)
{
    Q_D(LangSelect);
    d->source     = AcceptHeader;
    d->autoDetect = false;
}

LangSelect::~LangSelect() = default;

bool LangSelect::setup(Application *app)
{
    Q_D(LangSelect);

    const QVariantMap config = app->engine()->config(u"Cutelyst_LangSelect_Plugin"_qs);

    bool cookieExpirationOk = false;
    const QString cookieExpireStr =
        config.value(u"cookie_expiration"_qs, static_cast<qint64>(d->cookieExpiration.count()))
            .toString();
    d->cookieExpiration = std::chrono::duration_cast<std::chrono::seconds>(
        Utils::durationFromString(cookieExpireStr, &cookieExpirationOk));
    if (!cookieExpirationOk) {
        qCWarning(C_LANGSELECT).nospace() << "Invalid value set for cookie_expiration. "
                                             "Using default value "
#if QT_VERSION >= QT_VERSION_CHECK(6, 6, 0)
                                          << LangSelectPrivate::cookieDefaultExpiration;
#else
                                          << "1 month";
#endif
        d->cookieExpiration = LangSelectPrivate::cookieDefaultExpiration;
    }

    d->cookieDomain = config.value(u"cookie_domain"_qs).toString();

    const QString _sameSite = config.value(u"cookie_same_site"_qs, u"lax"_qs).toString();
    if (_sameSite.compare(u"default", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Default;
    } else if (_sameSite.compare(u"none", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::None;
    } else if (_sameSite.compare(u"stric", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Strict;
    } else if (_sameSite.compare(u"lax", Qt::CaseInsensitive) == 0) {
        d->cookieSameSite = QNetworkCookie::SameSite::Lax;
    } else {
        qCWarning(C_LANGSELECT).nospace() << "Invalid value set for cookie_same_site. "
                                             "Using default value "
                                          << QNetworkCookie::SameSite::Lax;
        d->cookieSameSite = QNetworkCookie::SameSite::Lax;
    }

    d->cookieSecure = config.value(u"cookie_secure"_qs).toBool();

    if ((d->cookieSameSite == QNetworkCookie::SameSite::None) && !d->cookieSecure) {
        qCWarning(C_LANGSELECT) << "cookie_same_site has been set to None but cookie_secure is "
                                   "not set to true. Implicitely setting cookie_secure to true. "
                                   "Please check your configuration.";
        d->cookieSecure = true;
    }

    if (d->fallbackLocale.language() == QLocale::C) {
        qCCritical(C_LANGSELECT) << "We need a valid fallback locale.";
        return false;
    }
    if (d->autoDetect) {
        if (d->source < Fallback) {
            if (d->source == URLQuery && d->queryKey.isEmpty()) {
                qCCritical(C_LANGSELECT) << "Can not use url query as source with empty key name.";
                return false;
            } else if (d->source == Session && d->sessionKey.isEmpty()) {
                qCCritical(C_LANGSELECT) << "Can not use session as source with empty key name.";
                return false;
            } else if (d->source == Cookie && d->cookieName.isEmpty()) {
                qCCritical(C_LANGSELECT) << "Can not use cookie as source with empty cookie name.";
                return false;
            }
        } else {
            qCCritical(C_LANGSELECT) << "Invalid source.";
            return false;
        }
        connect(app, &Application::beforePrepareAction, this, [d](Context *c, bool *skipMethod) {
            d->beforePrepareAction(c, skipMethod);
        });
    }
    if (!d->locales.contains(d->fallbackLocale)) {
        d->locales.append(d->fallbackLocale);
    }
    connect(app, &Application::postForked, this, &LangSelectPrivate::_q_postFork);

    qCDebug(C_LANGSELECT) << "Initialized LangSelect plugin with the following settings:";
    qCDebug(C_LANGSELECT) << "Supported locales:" << d->locales;
    qCDebug(C_LANGSELECT) << "Fallback locale:" << d->fallbackLocale;
    qCDebug(C_LANGSELECT) << "Auto detection source:" << d->source;
    qCDebug(C_LANGSELECT) << "Detect from header:" << d->detectFromHeader;

    return true;
}

void LangSelect::setSupportedLocales(const QVector<QLocale> &locales)
{
    Q_D(LangSelect);
    d->locales.clear();
    d->locales.reserve(locales.size());
    for (const QLocale &l : locales) {
        if (Q_LIKELY(l.language() != QLocale::C)) {
            d->locales.push_back(l);
        } else {
            qCWarning(C_LANGSELECT)
                << "Can not add invalid locale" << l << "to the list of supported locales.";
        }
    }
}

void LangSelect::setSupportedLocales(const QStringList &locales)
{
    Q_D(LangSelect);
    d->locales.clear();
    d->locales.reserve(locales.size());
    for (const QString &l : locales) {
        QLocale locale(l);
        if (Q_LIKELY(locale.language() != QLocale::C)) {
            d->locales.push_back(locale);
        } else {
            qCWarning(C_LANGSELECT)
                << "Can not add invalid locale" << l << "to the list of supported locales.";
        }
    }
}

void LangSelect::addSupportedLocale(const QLocale &locale)
{
    if (Q_LIKELY(locale.language() != QLocale::C)) {
        Q_D(LangSelect);
        d->locales.push_back(locale);
    } else {
        qCWarning(C_LANGSELECT) << "Can not add invalid locale" << locale
                                << "to the list of supported locales.";
    }
}

void LangSelect::addSupportedLocale(const QString &locale)
{
    QLocale l(locale);
    if (Q_LIKELY(l.language() != QLocale::C)) {
        Q_D(LangSelect);
        d->locales.push_back(l);
    } else {
        qCWarning(C_LANGSELECT) << "Can not add invalid locale" << locale
                                << "to the list of supported locales.";
    }
}

void LangSelect::setLocalesFromDir(const QString &path,
                                   const QString &name,
                                   const QString &prefix,
                                   const QString &suffix)
{
    Q_D(LangSelect);
    d->locales.clear();
    if (Q_LIKELY(!path.isEmpty() && !name.isEmpty())) {
        const QDir dir(path);
        if (Q_LIKELY(dir.exists())) {
            const auto _pref     = prefix.isEmpty() ? u"."_qs : prefix;
            const auto _suff     = suffix.isEmpty() ? u".qm"_qs : suffix;
            const QString filter = name + _pref + u'*' + _suff;
            const auto files     = dir.entryInfoList({name}, QDir::Files);
            if (Q_LIKELY(!files.empty())) {
                d->locales.reserve(files.size());
                bool shrinkToFit = false;
                for (const QFileInfo &fi : files) {
                    const auto fn      = fi.fileName();
                    const auto prefIdx = fn.indexOf(_pref);
                    const auto locPart =
                        fn.mid(prefIdx + _pref.length(),
                               fn.length() - prefIdx - _suff.length() - _pref.length());
                    QLocale l(locPart);
                    if (Q_LIKELY(l.language() != QLocale::C)) {
                        d->locales.push_back(l);
                        qCDebug(C_LANGSELECT)
                            << "Added locale" << locPart << "to the list of supported locales.";
                    } else {
                        shrinkToFit = true;
                        qCWarning(C_LANGSELECT) << "Can not add invalid locale" << locPart
                                                << "to the list of supported locales.";
                    }
                }
                if (shrinkToFit) {
                    d->locales.squeeze();
                }
            } else {
                qCWarning(C_LANGSELECT)
                    << "Can not find translation files for" << filter << "in" << path;
            }
        } else {
            qCWarning(C_LANGSELECT) << "Can not set locales from not existing directory" << path;
        }
    } else {
        qCWarning(C_LANGSELECT) << "Can not set locales from dir with empty path or name.";
    }
}

void LangSelect::setLocalesFromDirs(const QString &path, const QString &name)
{
    Q_D(LangSelect);
    d->locales.clear();
    if (Q_LIKELY(!path.isEmpty() && !name.isEmpty())) {
        const QDir dir(path);
        if (Q_LIKELY(dir.exists())) {
            const auto dirs = dir.entryList(QDir::AllDirs);
            if (Q_LIKELY(!dirs.empty())) {
                d->locales.reserve(dirs.size());
                bool shrinkToFit = false;
                for (const QString &subDir : dirs) {
                    const QString relFn = subDir + u'/' + name;
                    if (dir.exists(relFn)) {
                        QLocale l(subDir);
                        if (Q_LIKELY(l.language() != QLocale::C)) {
                            d->locales.push_back(l);
                            qCDebug(C_LANGSELECT)
                                << "Added locale" << subDir << "to the list of supported locales.";
                        } else {
                            shrinkToFit = true;
                            qCWarning(C_LANGSELECT) << "Can not add invalid locale" << subDir
                                                    << "to the list of supported locales.";
                        }
                    } else {
                        shrinkToFit = true;
                    }
                }
                if (shrinkToFit) {
                    d->locales.squeeze();
                }
            }
        } else {
            qCWarning(C_LANGSELECT) << "Can not set locales from not existing directory" << path;
        }
    } else {
        qCWarning(C_LANGSELECT) << "Can not set locales from dirs with empty path or names.";
    }
}

QVector<QLocale> LangSelect::supportedLocales() const
{
    Q_D(const LangSelect);
    return d->locales;
}

void LangSelect::setQueryKey(const QString &key)
{
    Q_D(LangSelect);
    d->queryKey = key;
}

void LangSelect::setSessionKey(const QString &key)
{
    Q_D(LangSelect);
    d->sessionKey = key;
}

void LangSelect::setCookieName(const QByteArray &name)
{
    Q_D(LangSelect);
    d->cookieName = name;
}

void LangSelect::setSubDomainMap(const QMap<QString, QLocale> &map)
{
    Q_D(LangSelect);
    d->subDomainMap.clear();
    d->locales.clear();
    d->locales.reserve(map.size());
    auto i = map.constBegin();
    while (i != map.constEnd()) {
        if (i.value().language() != QLocale::C) {
            d->subDomainMap.insert(i.key(), i.value());
            d->locales.append(i.value());
        } else {
            qCWarning(C_LANGSELECT) << "Can not add invalid locale" << i.value() << "for subdomain"
                                    << i.key() << "to the subdomain map.";
        }
        ++i;
    }
    d->locales.squeeze();
}

void LangSelect::setDomainMap(const QMap<QString, QLocale> &map)
{
    Q_D(LangSelect);
    d->domainMap.clear();
    d->locales.clear();
    d->locales.reserve(map.size());
    auto i = map.constBegin();
    while (i != map.constEnd()) {
        if (Q_LIKELY(i.value().language() != QLocale::C)) {
            d->domainMap.insert(i.key(), i.value());
            d->locales.append(i.value());
        } else {
            qCWarning(C_LANGSELECT) << "Can not add invalid locale" << i.value() << "for domain"
                                    << i.key() << "to the domain map.";
        }
        ++i;
    }
    d->locales.squeeze();
}

void LangSelect::setFallbackLocale(const QLocale &fallback)
{
    Q_D(LangSelect);
    d->fallbackLocale = fallback;
}

void LangSelect::setDetectFromHeader(bool enabled)
{
    Q_D(LangSelect);
    d->detectFromHeader = enabled;
}

void LangSelect::setLanguageCodeStashKey(const QString &key)
{
    Q_D(LangSelect);
    if (Q_LIKELY(!key.isEmpty())) {
        d->langStashKey = key;
    } else {
        qCWarning(C_LANGSELECT) << "Can not set an empty key name for the language code stash key. "
                                   "Using current key name"
                                << d->langStashKey;
    }
}

void LangSelect::setLanguageDirStashKey(const QString &key)
{
    Q_D(LangSelect);
    if (Q_LIKELY(!key.isEmpty())) {
        d->dirStashKey = key;
    } else {
        qCWarning(C_LANGSELECT) << "Can not set an empty key name for the language direction stash "
                                   "key. Using current key name"
                                << d->dirStashKey;
    }
}

QVector<QLocale> LangSelect::getSupportedLocales()
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return {};
    }

    return lsp->supportedLocales();
}

bool LangSelect::fromUrlQuery(Context *c, const QString &key)
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return true;
    }

    const auto d    = lsp->d_ptr.get();
    const auto _key = !key.isEmpty() ? key : d->queryKey;
    if (!d->getFromQuery(c, _key)) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToQuery(c, _key);
        c->detach();
        return false;
    }
    d->setContentLanguage(c);

    return true;
}

bool LangSelect::fromSession(Context *c, const QString &key)
{
    bool foundInSession = false;

    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return foundInSession;
    }

    const auto d    = lsp->d_ptr.get();
    const auto _key = !key.isEmpty() ? key : d->sessionKey;
    foundInSession  = d->getFromSession(c, _key);
    if (!foundInSession) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToSession(c, _key);
    }
    d->setContentLanguage(c);

    return foundInSession;
}

bool LangSelect::fromCookie(Context *c, const QByteArray &name)
{
    bool foundInCookie = false;

    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return foundInCookie;
    }

    const auto d     = lsp->d_ptr.get();
    const auto _name = !name.isEmpty() ? name : d->cookieName;
    foundInCookie    = d->getFromCookie(c, _name);
    if (!foundInCookie) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToCookie(c, _name);
    }
    d->setContentLanguage(c);

    return foundInCookie;
}

bool LangSelect::fromSubDomain(Context *c, const QMap<QString, QLocale> &subDomainMap)
{
    bool foundInSubDomain = false;

    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return foundInSubDomain;
    }

    const auto d     = lsp->d_ptr.get();
    const auto _map  = !subDomainMap.empty() ? subDomainMap : d->subDomainMap;
    foundInSubDomain = d->getFromSubdomain(c, _map);
    if (!foundInSubDomain) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
    }

    d->setContentLanguage(c);

    return foundInSubDomain;
}

bool LangSelect::fromDomain(Context *c, const QMap<QString, QLocale> &domainMap)
{
    bool foundInDomain = false;

    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return foundInDomain;
    }

    const auto d    = lsp->d_ptr.get();
    const auto _map = !domainMap.empty() ? domainMap : d->domainMap;
    foundInDomain   = d->getFromDomain(c, _map);
    if (!foundInDomain) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
    }

    d->setContentLanguage(c);

    return foundInDomain;
}

bool LangSelect::fromPath(Context *c, const QString &locale)
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return true;
    }

    const auto d = lsp->d_ptr.get();
    const QLocale l(locale);
    if (l.language() != QLocale::C && d->locales.contains(l)) {
        qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in path";
        c->setLocale(l);
        d->setContentLanguage(c);
        return true;
    } else {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        auto uri             = c->req()->uri();
        auto pathParts       = uri.path().split(u'/');
        const auto localeIdx = pathParts.indexOf(locale);
        pathParts[localeIdx] = c->locale().bcp47Name().toLower();
        uri.setPath(pathParts.join(u'/'));
        qCDebug(C_LANGSELECT) << "Storing selected locale by redirecting to" << uri;
        c->res()->redirect(uri, Response::TemporaryRedirect);
        c->detach();
        return false;
    }
}

bool LangSelectPrivate::detectLocale(Context *c, LangSelect::Source _source, bool *skipMethod) const
{
    bool redirect = false;

    LangSelect::Source foundIn = LangSelect::Fallback;

    if (_source == LangSelect::Session) {
        if (getFromSession(c, sessionKey)) {
            foundIn = _source;
        }
    } else if (_source == LangSelect::Cookie) {
        if (getFromCookie(c, cookieName)) {
            foundIn = _source;
        }
    } else if (_source == LangSelect::URLQuery) {
        if (getFromQuery(c, queryKey)) {
            foundIn = _source;
        }
    } else if (_source == LangSelect::SubDomain) {
        if (getFromSubdomain(c, subDomainMap)) {
            foundIn = _source;
        }
    } else if (_source == LangSelect::Domain) {
        if (getFromDomain(c, domainMap)) {
            foundIn = _source;
        }
    }

    // could not find supported locale in specified source
    // falling back to Accept-Language header
    if (foundIn == LangSelect::Fallback && getFromHeader(c)) {
        foundIn = LangSelect::AcceptHeader;
    }

    if (foundIn == LangSelect::Fallback) {
        setFallback(c);
    }

    if (foundIn != _source) {
        if (_source == LangSelect::Session) {
            setToSession(c, sessionKey);
        } else if (_source == LangSelect::Cookie) {
            setToCookie(c, cookieName);
        } else if (_source == LangSelect::URLQuery) {
            setToQuery(c, queryKey);
            redirect = true;
            if (skipMethod) {
                *skipMethod = true;
            }
        }
    }

    if (!redirect) {
        setContentLanguage(c);
    }

    return redirect;
}

bool LangSelectPrivate::getFromQuery(Context *c, const QString &key) const
{
    const QLocale l(c->req()->queryParam(key));
    if (l.language() != QLocale::C && locales.contains(l)) {
        qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in url query key" << key;
        c->setLocale(l);
        return true;
    } else {
        qCDebug(C_LANGSELECT) << "Can not find supported locale in url query key" << key;
        return false;
    }
}

bool LangSelectPrivate::getFromCookie(Context *c, const QByteArray &cookie) const
{
    const QLocale l(QString::fromLatin1(c->req()->cookie(cookie)));
    if (l.language() != QLocale::C && locales.contains(l)) {
        qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in cookie name" << cookie;
        c->setLocale(l);
        return true;
    } else {
        qCDebug(C_LANGSELECT) << "Can no find supported locale in cookie value with name" << cookie;
        return false;
    }
}

bool LangSelectPrivate::getFromSession(Context *c, const QString &key) const
{
    const QLocale l = Cutelyst::Session::value(c, key).toLocale();
    if (l.language() != QLocale::C && locales.contains(l)) {
        qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in session key" << key;
        c->setLocale(l);
        return true;
    } else {
        qCDebug(C_LANGSELECT) << "Can not find supported locale in session value with key" << key;
        return false;
    }
}

bool LangSelectPrivate::getFromSubdomain(Context *c, const QMap<QString, QLocale> &map) const
{
    const auto domain = c->req()->uri().host();
    auto i            = map.constBegin();
    while (i != map.constEnd()) {
        if (domain.startsWith(i.key())) {
            qCDebug(C_LANGSELECT) << "Found valid locale" << i.value()
                                  << "in subdomain map for domain" << domain;
            c->setLocale(i.value());
            return true;
        }
        ++i;
    }

    const auto domainParts = domain.split(u'.', Qt::SkipEmptyParts);
    if (domainParts.size() > 2) {
        const QLocale l(domainParts.at(0));
        if (l.language() != QLocale::C && locales.contains(l)) {
            qCDebug(C_LANGSELECT) << "Found supported locale" << l << "in subdomain of domain"
                                  << domain;
            c->setLocale(l);
            return true;
        }
    }
    qCDebug(C_LANGSELECT) << "Can not find supported locale for subdomain" << domain;
    return false;
}

bool LangSelectPrivate::getFromDomain(Context *c, const QMap<QString, QLocale> &map) const
{
    const auto domain = c->req()->uri().host();
    auto i            = map.constBegin();
    while (i != map.constEnd()) {
        if (domain.endsWith(i.key())) {
            qCDebug(C_LANGSELECT) << "Found valid locale" << i.value() << "in domain map for domain"
                                  << domain;
            c->setLocale(i.value());
            return true;
        }
        ++i;
    }

    const auto domainParts = domain.split(u'.', Qt::SkipEmptyParts);
    if (domainParts.size() > 1) {
        const QLocale l(domainParts.at(domainParts.size() - 1));
        if (l.language() != QLocale::C && locales.contains(l)) {
            qCDebug(C_LANGSELECT) << "Found supported locale" << l << "in domain" << domain;
            c->setLocale(l);
            return true;
        }
    }
    qCDebug(C_LANGSELECT) << "Can not find supported locale for domain" << domain;
    return false;
}

bool LangSelectPrivate::getFromHeader(Context *c, const QByteArray &name) const
{
    if (detectFromHeader) {
        // TODO Qt::SkipEmptyParts
        const auto accpetedLangs = c->req()->header(name).split(',');
        if (Q_LIKELY(!accpetedLangs.empty())) {
            std::map<float, QLocale> langMap;
            for (const auto &ba : accpetedLangs) {
                const QString al = QString::fromLatin1(ba);
                const auto idx   = al.indexOf(u';');
                float priority   = 1.0f;
                QString langPart;
                bool ok = true;
                if (idx > -1) {
                    langPart       = al.left(idx);
                    const auto ref = QStringView(al).mid(idx + 1);
                    priority       = ref.mid(ref.indexOf(u'=') + 1).toFloat(&ok);
                } else {
                    langPart = al;
                }
                QLocale locale(langPart);
                if (ok && locale.language() != QLocale::C) {
                    const auto search = langMap.find(priority);
                    if (search == langMap.cend()) {
                        langMap.insert({priority, locale});
                    }
                }
            }
            if (!langMap.empty()) {
                auto i = langMap.crbegin();
                while (i != langMap.crend()) {
                    if (locales.contains(i->second)) {
                        c->setLocale(i->second);
                        qCDebug(C_LANGSELECT)
                            << "Selected locale" << c->locale() << "from" << name << "header";
                        return true;
                    }
                    ++i;
                }
                // if there is no exact match, lets try to find a locale
                // where at least the language matches
                i                       = langMap.crbegin();
                const auto constLocales = locales;
                while (i != langMap.crend()) {
                    for (const QLocale &l : constLocales) {
                        if (l.language() == i->second.language()) {
                            c->setLocale(l);
                            qCDebug(C_LANGSELECT)
                                << "Selected locale" << c->locale() << "from" << name << "header";
                            return true;
                        }
                    }
                    ++i;
                }
            }
        }
    }

    return false;
}

void LangSelectPrivate::setToQuery(Context *c, const QString &key) const
{
    auto uri = c->req()->uri();
    QUrlQuery query(uri);
    if (query.hasQueryItem(key)) {
        query.removeQueryItem(key);
    }
    query.addQueryItem(key, c->locale().bcp47Name().toLower());
    uri.setQuery(query);
    qCDebug(C_LANGSELECT) << "Storing selected" << c->locale() << "in URL query by redirecting to"
                          << uri;
    c->res()->redirect(uri, Response::TemporaryRedirect);
}

void LangSelectPrivate::setToCookie(Context *c, const QByteArray &name) const
{
    qCDebug(C_LANGSELECT) << "Storing selected" << c->locale() << "in cookie with name" << name;
    QNetworkCookie cookie(name, c->locale().bcp47Name().toLatin1());
    cookie.setSameSitePolicy(QNetworkCookie::SameSite::Lax);
    if (cookieExpiration.count() == 0) {
        cookie.setExpirationDate(QDateTime());
    } else {
#if QT_VERSION >= QT_VERSION_CHECK(6, 4, 0)
        cookie.setExpirationDate(QDateTime::currentDateTime().addDuration(cookieExpiration));
#else
        cookie.setExpirationDate(QDateTime::currentDateTime().addSecs(cookieExpiration.count()));
#endif
    }
    cookie.setDomain(cookieDomain);
    cookie.setSecure(cookieSecure);
    cookie.setSameSitePolicy(cookieSameSite);
    c->res()->setCookie(cookie);
}

void LangSelectPrivate::setToSession(Context *c, const QString &key) const
{
    qCDebug(C_LANGSELECT) << "Storing selected" << c->locale() << "in session key" << key;
    Session::setValue(c, key, c->locale());
}

void LangSelectPrivate::setFallback(Context *c) const
{
    qCDebug(C_LANGSELECT) << "Can not find fitting locale, using fallback locale" << fallbackLocale;
    c->setLocale(fallbackLocale);
}

void LangSelectPrivate::setContentLanguage(Context *c) const
{
    if (addContentLanguageHeader) {
        c->res()->setHeader("Content-Language"_qba, c->locale().bcp47Name().toLatin1());
    }
    c->stash(
        {{langStashKey, c->locale().bcp47Name()},
         {dirStashKey, (c->locale().textDirection() == Qt::LeftToRight ? u"ltr"_qs : u"rtl"_qs)}});
}

void LangSelectPrivate::beforePrepareAction(Context *c, bool *skipMethod) const
{
    if (*skipMethod) {
        return;
    }

    if (!c->stash(LangSelectPrivate::stashKeySelectionTried).isNull()) {
        return;
    }

    detectLocale(c, source, skipMethod);

    c->setStash(LangSelectPrivate::stashKeySelectionTried, true);
}

void LangSelectPrivate::_q_postFork(Application *app)
{
    lsp = app->plugin<LangSelect *>();
}

#include "moc_langselect.cpp"
