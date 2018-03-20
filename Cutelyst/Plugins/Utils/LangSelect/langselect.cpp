/*
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#include "langselect_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Response>

#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QUrl>
#include <QNetworkCookie>
#include <QUrlQuery>

#include <map>
#include <utility>

Q_LOGGING_CATEGORY(C_LANGSELECT, "cutelyst.plugin.langselect")

using namespace Cutelyst;

static thread_local LangSelect *lsp = nullptr;

#define SELECTION_TRIED QLatin1String("_c_langselect_tried")

LangSelect::LangSelect(Application *parent, Cutelyst::LangSelect::Source source) :
    Plugin(parent), d_ptr(new LangSelectPrivate)
{
    Q_D(LangSelect);
    d->source = source;
    d->autoDetect = true;
}

LangSelect::LangSelect(Application *parent) :
    Plugin(parent), d_ptr(new LangSelectPrivate)
{
    Q_D(LangSelect);
    d->source = AcceptHeader;
    d->autoDetect = false;
}


LangSelect::~LangSelect()
{
    delete d_ptr;
}

bool LangSelect::setup(Application *app)
{
    Q_D(LangSelect);
    if (d->fallbackLocale.language() == QLocale::C) {
        qCCritical(C_LANGSELECT, "We need a valid fallback locale.");
        return false;
    }
    if (d->autoDetect) {
        if (d->source < Fallback) {
            if (d->source == URLQuery && d->queryKey.isEmpty()) {
                qCCritical(C_LANGSELECT, "Can not use url query as source with empty key name.");
                return false;
            } else if (d->source == Session && d->sessionKey.isEmpty()) {
                qCCritical(C_LANGSELECT, "Can not use session as source with empty key name.");
                return false;
            } else if (d->source == Cookie && d->cookieName.isEmpty()) {
                qCCritical(C_LANGSELECT, "Can not use cookie as source with empty cookie name.");
                return false;
            } else if (d->source == SubDomain && d->subDomainIdx < 0) {
                qCCritical(C_LANGSELECT, "Can not use subdomain as source with invalid index.");
                return false;
            } else if (d->source == Domain && d->domainMap.empty()) {
                qCCritical(C_LANGSELECT, "Can not use domain as source with empty domain map.");
                return false;
            } else if (d->source == Path && d->pathIdx < 0) {
                qCCritical(C_LANGSELECT, "Can not use path as source with invalid index.");
                return false;
            }
        } else {
            qCCritical(C_LANGSELECT, "Invalid source.");
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

    return true;
}

void LangSelect::setSupportedLocales(const QVector<QLocale> &locales)
{
    Q_D(LangSelect);
    d->locales = locales;
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
            qCWarning(C_LANGSELECT, "Can not add invalid locale \"%s\" to the list of supported locales.", qUtf8Printable(l));
        }
    }
}

void LangSelect::addSupportedLocale(const QLocale &locale)
{
    Q_D(LangSelect);
    d->locales.push_back(locale);
}

void LangSelect::addSupportedLocale(const QString &locale)
{
    QLocale l(locale);
    if (l.language() != QLocale::C) {
        Q_D(LangSelect);
        d->locales.push_back(l);
    } else {
        qCWarning(C_LANGSELECT, "Can not add invalid locale \"%s\" to the list of supported locales.", qUtf8Printable(locale));
    }
}

void LangSelect::setLocalesFromDir(const QString &path, const QString &name, const QString prefix, const QString &suffix)
{
    Q_D(LangSelect);
    d->locales.clear();
    if (Q_LIKELY(!path.isEmpty() && !name.isEmpty())) {
        const QDir dir(path);
        if (Q_LIKELY(dir.exists())) {
            const auto _pref = prefix.isEmpty() ? QStringLiteral(".") : prefix;
            const auto _suff = suffix.isEmpty() ? QStringLiteral(".qm") : suffix;
            const QString filter = name + _pref + QLatin1Char('*') + _suff;
            const auto files = dir.entryInfoList({name}, QDir::Files);
            if (Q_LIKELY(!files.empty())) {
                d->locales.reserve(files.size());
                bool shrinkToFit = false;
                for (const QFileInfo &fi : files) {
                    const auto fn = fi.fileName();
                    const auto prefIdx = fn.indexOf(_pref);
                    const auto locPart = fn.mid(prefIdx + _pref.length(), fn.length() - prefIdx - _suff.length() - _pref.length());
                    QLocale l(locPart);
                    if (Q_LIKELY(l.language() != QLocale::C)) {
                        d->locales.push_back(l);
                        qCDebug(C_LANGSELECT, "Added locale \"%s\" to the list of supported locales.", qUtf8Printable(locPart));
                    } else {
                        shrinkToFit = true;
                        qCWarning(C_LANGSELECT, "Can not add invalid locale \"%s\" to the list of supported locales.", qUtf8Printable(locPart));
                    }
                }
                if (shrinkToFit) {
                    d->locales.shrink_to_fit();
                }
            } else {
                qCWarning(C_LANGSELECT, "Can not find translation files for \"%s\" in \"%s\".", qUtf8Printable(filter), qUtf8Printable(path));
            }
        } else {
            qCWarning(C_LANGSELECT, "Can not set locales from not existing directory \"%s\".", qUtf8Printable(path));
        }
    } else {
        qCWarning(C_LANGSELECT, "Can not set locales from dir with emtpy path or name.");
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
                    const QString relFn = subDir + QLatin1Char('/') + name;
                    if (dir.exists(relFn)) {
                        QLocale l(subDir);
                        if (Q_LIKELY(l.language() != QLocale::C)) {
                            d->locales.push_back(l);
                            qCDebug(C_LANGSELECT, "Added locale \"%s\" to the list of supported locales.", qUtf8Printable(subDir));
                        }  else {
                            shrinkToFit = true;
                            qCWarning(C_LANGSELECT, "Can not add invalid locale \"%s\" to the list of supported locales.");
                        }
                    } else {
                        shrinkToFit = true;
                    }
                }
                if (shrinkToFit) {
                    d->locales.shrink_to_fit();
                }
            }
        } else {
            qCWarning(C_LANGSELECT, "Can not set locales from not existing directory \"%s\".", qUtf8Printable(path));
        }
    } else {
        qCWarning(C_LANGSELECT, "Can not set locales from dirs with empty path or names.");
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

void LangSelect::setCookieName(const QString &name)
{
    Q_D(LangSelect);
    d->cookieName = name;
}

void LangSelect::setSubDomainIndex(qint8 index)
{
    Q_D(LangSelect);
    d->subDomainIdx = index;
}

void LangSelect::setPathIndex(qint8 index)
{
    Q_D(LangSelect);
    d->pathIdx = index;
}

void LangSelect::setDomainMap(const QMap<QString, QLocale> &map)
{
    Q_D(LangSelect);
    d->domainMap = map;
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

QVector<QLocale> LangSelect::getSupportedLocales()
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return QVector<QLocale>();
    }

    return lsp->supportedLocales();
}

bool LangSelect::fromUrlQuery(Context *c, const QString &key)
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return true;
    }

    const auto d = lsp->d_ptr;
    const auto _key = !key.isEmpty() ? key : d->queryKey;
    if (!d->getFromQuery(c, _key)) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToQuery(c, _key);
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

    const auto d = lsp->d_ptr;
    const auto _key = !key.isEmpty() ? key : d->sessionKey;
    foundInSession = d->getFromSession(c, _key);
    if (!foundInSession) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToSession(c, _key);
    }
    d->setContentLanguage(c);

    return foundInSession;
}

bool LangSelect::fromCookie(Context *c, const QString &name)
{
    bool foundInCookie = false;

    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return foundInCookie;
    }

    const auto d = lsp->d_ptr;
    const auto _name = !name.isEmpty() ? name : d->cookieName;
    foundInCookie = d->getFromCookie(c, _name);
    if (!foundInCookie) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToCookie(c, _name);
    }
    d->setContentLanguage(c);

    return foundInCookie;
}

bool LangSelect::fromSubDomain(Context *c, qint8 subDomainIndex)
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return false;
    }

    const auto d = lsp->d_ptr;
    const auto idx = (subDomainIndex > 127) ? subDomainIndex : d->subDomainIdx;
    bool foundValidSubdomain = false;
    if (!d->getFromSubdomain(c, idx, &foundValidSubdomain)) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToSubdomain(c, idx, foundValidSubdomain);
        return true;
    }
    d->setContentLanguage(c);

    return false;
}

bool LangSelect::fromDomain(Context *c, const QMap<QString,QLocale> &domainMap)
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return false;
    }

    const auto d = lsp->d_ptr;
    const auto _map = !domainMap.empty() ? domainMap : d->domainMap;
    if (!d->getFromDomain(c, _map)) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToDomain(c, _map);
        return true;
    }

    d->setContentLanguage(c);

    return false;
}

bool LangSelect::fromPath(Context *c, qint8 pathIndex)
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return false;
    }

    const auto d = lsp->d_ptr;
    const auto idx = (pathIndex > -127) ? pathIndex : d->pathIdx;
    bool foundValidLocale = false;
    if (!d->getFromPath(c, idx, &foundValidLocale)) {
        if (!d->getFromHeader(c)) {
            d->setFallback(c);
        }
        d->setToPath(c, idx, foundValidLocale);
        return true;
    }

    d->setContentLanguage(c);

    return false;
}

bool LangSelectPrivate::detectLocale(Context *c, LangSelect::Source _source, bool *skipMethod) const
{
    bool redirect = false;

    LangSelect::Source foundIn = LangSelect::Fallback;
    bool pathContainedValidLocale = false;
    bool subDomainContainedValidLocale = false;

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
    } else if (_source == LangSelect::Path) {
        if (getFromPath(c, pathIdx, &pathContainedValidLocale)) {
            foundIn = _source;
        }
    } else if (_source == LangSelect::SubDomain) {
        if (getFromSubdomain(c, subDomainIdx, &subDomainContainedValidLocale)) {
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
        if (_source == LangSelect::Path) {
            setToPath(c, pathIdx, pathContainedValidLocale);
            redirect = true;
            if (skipMethod) {
                *skipMethod = true;
            }
        } else if (_source == LangSelect::SubDomain) {
            setToSubdomain(c, subDomainIdx, subDomainContainedValidLocale);
            redirect = true;
            if (skipMethod) {
                *skipMethod = true;
            }
        } else if (_source == LangSelect::Session) {
            setToSession(c, sessionKey);
        } else if (_source == LangSelect::Cookie) {
            setToCookie(c, cookieName);
        } else if (_source == LangSelect::Domain) {
            setToDomain(c, domainMap);
            redirect = true;
            if (skipMethod) {
                *skipMethod = true;
            }
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

bool LangSelectPrivate::getFromCookie(Context *c, const QString &cookie) const
{
    const QLocale l(c->req()->cookie(cookie));
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
    const QLocale l = Cutelyst::Session::value(c, key).value<QLocale>();
    if (l.language() != QLocale::C && locales.contains(l)) {
        qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in session key" << key;
        c->setLocale(l);
        return true;
    } else {
        qCDebug(C_LANGSELECT) << "Can not find supported locale in session value with key" << key;
        return false;
    }
}

bool LangSelectPrivate::getFromPath(Context *c, int idx, bool *pathContainedValidLocale) const
{
    if (Q_LIKELY(idx >= 0)) {
        const auto pathParts = c->req()->uri().path().split(QLatin1Char('/'), QString::SkipEmptyParts);
        if (Q_LIKELY(idx < pathParts.size())) {
            QLocale l(pathParts.at(idx));
            if (l.language() != QLocale::C) {
                *pathContainedValidLocale = true;
                if (locales.contains(l)) {
                    qCDebug(C_LANGSELECT) << "Found valid locale" << l << "at url path index" << idx;
                    c->setLocale(l);
                    return true;
                }
            }
            qCDebug(C_LANGSELECT) << "Can not find supported locale at path part index" << idx;
        } else {
            qCWarning(C_LANGSELECT, "Path part index %i is not below the number of path parts of %i.", idx, pathParts.size());
        }
    } else {
        qCWarning(C_LANGSELECT, "Can not select locale from invalid path part index.");
    }
    return false;
}

bool LangSelectPrivate::getFromSubdomain(Context *c, int idx, bool *subDomainContainedValidLocale) const
{
    if (Q_LIKELY(idx >= 0)) {
        const auto hostParts = c->req()->uri().host().split(QLatin1Char('.'), QString::SkipEmptyParts);
        if (hostParts.size() > 2) {
            if (idx < (hostParts.size() - 2)) {
                QLocale l(hostParts.at(idx));
                if (l.language() != QLocale::C) {
                    *subDomainContainedValidLocale = true;
                    if (locales.contains(l)) {
                        qCDebug(C_LANGSELECT) << "Found valid locale" << l << "at domain part index" << idx;
                        c->setLocale(l);
                        return true;
                    }
                }
            }
            qCDebug(C_LANGSELECT) << "Can not find supported locale at subdomain part index" << idx;
        } else {
            qCWarning(C_LANGSELECT, "Subdomain part index %i is not below the number of subdomain parts of %i.", idx, hostParts.size());
        }
    } else {
        qCWarning(C_LANGSELECT, "Can not select locale from invalid subdomain part index.");
    }
    return false;
}

bool LangSelectPrivate::getFromDomain(Context *c, const QMap<QString, QLocale> &map) const
{
    if (Q_LIKELY(!map.empty())) {
        const auto domain = c->req()->uri().host();
        auto i = map.constBegin();
        while (i != map.constEnd()) {
            if (domain.endsWith(i.key())) {
                qCDebug(C_LANGSELECT) << "Found valid locale" << i.value() << "in domain map for domain" << domain;
                c->setLocale(i.value());
                return true;
            }
            ++i;
        }
        qCDebug(C_LANGSELECT) << "Can not find supported locale for domain" << domain;
    } else {
        qCWarning(C_LANGSELECT, "Can not select locale from empty domain map.");
    }
    return false;
}

bool LangSelectPrivate::getFromHeader(Context *c, const QString &name) const
{
    if (detectFromHeader) {
        const auto accpetedLangs = c->req()->header(name).split(QLatin1Char(','), QString::SkipEmptyParts);
        if (Q_LIKELY(!accpetedLangs.empty())) {
            std::map<float,QLocale> langMap;
            for (const QString &al : accpetedLangs) {
                const auto idx = al.indexOf(QLatin1Char(';'));
                float priority = 1.0f;
                QString langPart;
                bool ok = true;
                if (idx > -1) {
                    langPart = al.left(idx);
                    const QStringRef ref = al.midRef(idx + 1);
                    priority = ref.mid(ref.indexOf(QLatin1Char('=')) +1).toFloat(&ok);
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
                        qCDebug(C_LANGSELECT) << "Selected locale" << c->locale() << "from" << name << "header";
                        return true;
                    }
                    ++i;
                }
                // if there is no exact match, lets try to find a locale
                // where at least the language matches
                i = langMap.crbegin();
                const auto constLocales = locales;
                while (i != langMap.crend()) {
                    for (const QLocale &l : constLocales) {
                        if (l.language() == i->second.language()) {
                            c->setLocale(i->second);
                            qCDebug(C_LANGSELECT) << "Selected locale" << c->locale() << "from" << name << "header";
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
    query.addQueryItem(key, c->locale().bcp47Name());
    uri.setQuery(query);
    qCDebug(C_LANGSELECT) << "Storing selected locale in URL query by redirecting to" << uri;
    c->res()->redirect(uri, 307);
}

void LangSelectPrivate::setToCookie(Context *c, const QString &name) const
{
    qCDebug(C_LANGSELECT) << "Storing selected locale in cookie with name" << name;
    c->res()->setCookie(QNetworkCookie(name.toLatin1(), c->locale().bcp47Name().toLatin1()));
}

void LangSelectPrivate::setToSession(Context *c, const QString &key) const
{
    qCDebug(C_LANGSELECT) << "Storing selected locale in session key" << sessionKey;
    Session::setValue(c, key, c->locale());
}

void LangSelectPrivate::setToPath(Context *c, int idx, bool pathContainedValidLocale) const
{
    auto uri = c->req()->uri();
    auto pathParts = uri.path().split(QLatin1Char('/'), QString::SkipEmptyParts);
    if (pathContainedValidLocale) {
        pathParts[idx] = c->locale().bcp47Name();
    } else {
        pathParts.insert(idx, c->locale().bcp47Name());
    }
    uri.setPath(QLatin1Char('/') + pathParts.join(QLatin1Char('/')));
    qCDebug(C_LANGSELECT) << "Storing selected locale in path by redirecting to" << uri;
    c->res()->redirect(uri, 307);
}

void LangSelectPrivate::setToSubdomain(Context *c, int idx, bool subDomainContainedValidLocale) const
{
    auto uri = c->req()->uri();
    auto hostParts = uri.host().split(QLatin1Char('.'), QString::SkipEmptyParts);
    if (subDomainContainedValidLocale) {
        hostParts[idx] = c->locale().bcp47Name();
    } else {
        hostParts.insert(idx, c->locale().bcp47Name());
    }
    uri.setHost(hostParts.join(QLatin1Char('.')));
    qCDebug(C_LANGSELECT) << "Storing selected locale in subdomain by redirecting to" << uri;
    c->res()->redirect(uri, 307);
}

void LangSelectPrivate::setToDomain(Context *c, const QMap<QString, QLocale> &map) const
{
    const auto langDomainParts = map.key(c->locale()).split(QLatin1Char('.'), QString::SkipEmptyParts);
    auto uri = c->req()->uri();
    auto currDomainParts = uri.host().split(QLatin1Char('.'), QString::SkipEmptyParts);
    if (currDomainParts <= langDomainParts) {
        uri.setHost(langDomainParts.join(QLatin1Char('.')));
    } else {
        for (int i = 0; i < langDomainParts.size(); ++i) {
            currDomainParts.takeLast();
        }
        currDomainParts.append(langDomainParts);
        uri.setHost(currDomainParts.join(QLatin1Char('.')));
    }
    qCDebug(C_LANGSELECT) << "Storing selected locale in domain by redirecting to" << uri;
    c->res()->redirect(uri, 307);
}

void LangSelectPrivate::setFallback(Context *c) const
{
    qCDebug(C_LANGSELECT) << "Can not find fitting locale, using fallback locale" << fallbackLocale;
    c->setLocale(fallbackLocale);
}

void LangSelectPrivate::setContentLanguage(Context *c) const
{
    if (addContentLanguageHeader) {
        c->res()->setHeader(QStringLiteral("Content-Language"), c->locale().bcp47Name());
    }
}

void LangSelectPrivate::beforePrepareAction(Context *c, bool *skipMethod) const
{
    if (*skipMethod) {
        return;
    }

    if (!c->stash(SELECTION_TRIED).isNull()) {
        return;
    }

    detectLocale(c, source, skipMethod);

    c->setStash(SELECTION_TRIED, true);
}

void LangSelectPrivate::_q_postFork(Application *app)
{
    lsp = app->plugin<LangSelect *>();
}

#include "moc_langselect.cpp"
