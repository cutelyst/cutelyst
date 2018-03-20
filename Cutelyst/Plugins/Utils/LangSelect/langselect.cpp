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

#include <map>
#include <utility>

Q_LOGGING_CATEGORY(C_LANGSELECT, "cutelyst.plugin.langselect")

using namespace Cutelyst;

static thread_local LangSelect *lsp = nullptr;

#define SELECTION_TRIED QLatin1String("_c_langselect_tried")

LangSelect::LangSelect(Application *parent, bool autoDetect) :
    Plugin(parent), d_ptr(new LangSelectPrivate)
{
    Q_D(LangSelect);
    d->autoDetect = autoDetect;
}

LangSelect::~LangSelect()
{
    delete d_ptr;
}

bool LangSelect::setup(Application *app)
{
    Q_D(LangSelect);
    if (!d->sourceOrder.empty()) {
        d->storeTo = d->sourceOrder.at(0);
    }
    if (!d->locales.contains(d->fallbackLocale)) {
        d->locales.append(d->fallbackLocale);
    }
    connect(app, &Application::postForked, this, &LangSelectPrivate::_q_postFork);
    if (d->autoDetect) {
        connect(app, &Application::beforePrepareAction, this, [d](Context *c, bool *skipMethod) {
            d->beforePrepareAction(c, skipMethod);
        });
    }

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
    d->sourceOrder.removeAll(URLQuery);
    if (!key.isEmpty()) {
        d->sourceOrder.push_back(URLQuery);
    }
}

void LangSelect::setSessionKey(const QString &key)
{
    Q_D(LangSelect);
    d->sessionKey = key;
    d->sourceOrder.removeAll(Session);
    if (!key.isEmpty()) {
        d->sourceOrder.push_back(Session);
    }
}

void LangSelect::setCookieName(const QString &name)
{
    Q_D(LangSelect);
    d->cookieName = name;
    d->sourceOrder.removeAll(Cookie);
    if (!name.isEmpty()) {
        d->sourceOrder.push_back(Cookie);
    }
}

void LangSelect::setSubDomainIndex(qint8 index)
{
    Q_D(LangSelect);
    d->subDomainIdx = index;
    d->sourceOrder.removeAll(SubDomain);
    if (index >= 0) {
        d->sourceOrder.push_back(SubDomain);
    }
}

void LangSelect::setPathIndex(qint8 index)
{
    Q_D(LangSelect);
    d->pathIdx = index;
    d->sourceOrder.removeAll(Path);
    if (index >= 0) {
        d->sourceOrder.push_back(Path);
    }
}

void LangSelect::setDomainMap(const QMap<QString, QLocale> &map)
{
    Q_D(LangSelect);
    d->domainMap = map;
    d->sourceOrder.removeAll(Domain);
    if (!map.empty()) {
        d->sourceOrder.push_back(Domain);
    }
}

void LangSelect::setSourceOrder(const QVector<Source> &order)
{
    Q_D(LangSelect);
    d->sourceOrder.clear();
    if (!order.empty()) {
        d->sourceOrder.reserve(order.size());
        for (Source s : order) {
            if (s == URLQuery && !d->queryKey.isEmpty()) {
                d->sourceOrder.push_back(s);
            } else if (s == Session && !d->sessionKey.isEmpty()) {
                d->sourceOrder.push_back(s);
            } else if (s == Cookie && !d->cookieName.isEmpty()) {
                d->sourceOrder.push_back(s);
            } else if (s == SubDomain && d->subDomainIdx >= 0) {
                d->sourceOrder.push_back(s);
            } else if (s == Path && d->pathIdx >= 0) {
                d->sourceOrder.push_back(s);
            } else if (s == Domain && !d->domainMap.empty()) {
                d->sourceOrder.push_back(s);
            }
        }
    }
}

void LangSelect::setFallbackLocale(const QLocale &fallback)
{
    Q_D(LangSelect);
    d->fallbackLocale = fallback;
}

QVector<QLocale> LangSelect::getSupportedLocales()
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return QVector<QLocale>();
    }

    return lsp->supportedLocales();
}

bool LangSelect::select(Context *c, const QVector<Cutelyst::LangSelect::Source> &sourceOrder)
{
    if (!lsp) {
        qCCritical(C_LANGSELECT) << "LangSelect plugin not registered";
        return false;
    }

    return lsp->d_ptr->detectLocale(c, sourceOrder);
}

bool LangSelectPrivate::detectLocale(Context *c, const QVector<LangSelect::Source> &_sourceOrder, bool *skipMethod)
{
    bool redirect = false;

    if (!c->stash(SELECTION_TRIED).isNull()) {
        return redirect;
    }

    LangSelect::Source foundIn = LangSelect::Fallback;
    if (Q_UNLIKELY(locales.empty())) {
        c->setLocale(fallbackLocale);
    }

    bool pathContainedValidLocale = false;
    bool subDomainContainedValidLocale = false;
    for (LangSelect::Source s : _sourceOrder) {
        if (s == LangSelect::Session) {
            const QLocale l = Session::value(c, sessionKey).value<QLocale>();
            if (l.language() != QLocale::C && locales.contains(l)) {
                qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in session key" << sessionKey;
                c->setLocale(l);
                foundIn = s;
                break;
            }
        } else if (s == LangSelect::Cookie) {
            QLocale l(c->req()->cookie(cookieName));
            if (l.language() != QLocale::C && locales.contains(l)) {
                qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in cookie name" << cookieName;
                c->setLocale(l);
                foundIn = s;
                break;
            }
        } else if (s == LangSelect::URLQuery) {
            QLocale l(c->req()->queryParam(queryKey));
            if (l.language() != QLocale::C && locales.contains(l)) {
                qCDebug(C_LANGSELECT) << "Found valid locale" << l << "in url query key" << queryKey;
                c->setLocale(l);
                foundIn = s;
                break;
            }
        } else if (s == LangSelect::Path) {
            const auto pathParts = c->req()->uri().path().split(QLatin1Char('/'), QString::SkipEmptyParts);
            if (pathIdx < pathParts.size()) {
                QLocale l(pathParts.at(pathIdx));
                if (l.language() != QLocale::C) {
                    pathContainedValidLocale = true;
                    if (locales.contains(l)) {
                        qCDebug(C_LANGSELECT) << "Found valid locale" << l << "at url path index" << pathIdx;
                        c->setLocale(l);
                        foundIn = s;
                        break;
                    }
                }
            }
        } else if (s == LangSelect::SubDomain) {
            const auto hostParts = c->req()->uri().host().split(QLatin1Char('.'), QString::SkipEmptyParts);
            if (hostParts.size() > 2) {
                if (subDomainIdx < (hostParts.size() - 2)) {
                    QLocale l(hostParts.at(subDomainIdx));
                    if (l.language() != QLocale::C) {
                        subDomainContainedValidLocale = true;
                        if (locales.contains(l)) {
                            qCDebug(C_LANGSELECT) << "Found valid locale" << l << "at domain part index" << subDomainIdx;
                            c->setLocale(l);
                            foundIn = s;
                            break;
                        }
                    }
                }
            }
        } else if (s == LangSelect::Domain) {
            const auto domain = c->req()->uri().host();
            auto i = domainMap.constBegin();
            while (i != domainMap.constEnd()) {
                if (domain.endsWith(i.key())) {
                    qCDebug(C_LANGSELECT) << "Found valid locale" << i.value() << "in domain map for domain" << domain;
                    c->setLocale(i.value());
                    foundIn = s;
                    break;
                    ++i;
                }
            }
        }
    }

    if (foundIn == LangSelect::Fallback) {
        const auto accpetedLangs = c->req()->header(QStringLiteral("Accept-Language")).split(QLatin1Char(','), QString::SkipEmptyParts);
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
                        foundIn = LangSelect::AcceptHeader;
                        c->setLocale(i->second);
                        qCDebug(C_LANGSELECT) << "Selected locale" << c->locale() << "from Accept-Language header";
                        break;
                    }
                    ++i;
                }
                // if there is no exact match, lets try to find a locale
                // where at least the language matches
                if (foundIn == LangSelect::Fallback) {
                    i = langMap.crbegin();
                    const auto constLocales = locales;
                    while (i != langMap.crend()) {
                        bool found = false;
                        for (const QLocale &l : constLocales) {
                            if (l.language() == i->second.language()) {
                                found = true;
                                foundIn = LangSelect::AcceptHeader;
                                c->setLocale(i->second);
                                qCDebug(C_LANGSELECT) << "Selected locale" << c->locale() << "from Accept-Language header";
                                break;
                            }
                        }
                        if (found) {
                            break;
                        }
                        ++i;
                    }
                }
            }
        }
    }

    if (foundIn == LangSelect::Fallback) {
        qCDebug(C_LANGSELECT) << "Can not find fitting locale, using fallback locale" << fallbackLocale;
        c->setLocale(fallbackLocale);
    }

    if (foundIn != storeTo) {
        if (storeTo == LangSelect::Path) {
            auto uri = c->req()->uri();
            auto pathParts = uri.path().split(QLatin1Char('/'), QString::SkipEmptyParts);
            if (pathContainedValidLocale) {
                pathParts[pathIdx] = c->locale().bcp47Name();
            } else {
                pathParts.insert(pathIdx, c->locale().bcp47Name());
            }
            uri.setPath(QLatin1Char('/') + pathParts.join(QLatin1Char('/')));
            qCDebug(C_LANGSELECT) << "Storing selected locale in path by redirecting to" << uri;
            c->res()->redirect(uri, 307);
            redirect = true;
            if (skipMethod) {
                *skipMethod = true;
            }
        } else if (storeTo == LangSelect::SubDomain) {
            auto uri = c->req()->uri();
            auto hostParts = uri.host().split(QLatin1Char('.'), QString::SkipEmptyParts);
            if (subDomainContainedValidLocale) {
                hostParts[subDomainIdx] = c->locale().bcp47Name();
            } else {
                hostParts.insert(subDomainIdx, c->locale().bcp47Name());
            }
            uri.setHost(hostParts.join(QLatin1Char('.')));
            qCDebug(C_LANGSELECT) << "Storing selected locale in subdomain by redirecting to" << uri;
            c->res()->redirect(uri, 307);
            redirect = true;
            if (skipMethod) {
                *skipMethod = true;
            }
        } else if (storeTo == LangSelect::Session) {
            qCDebug(C_LANGSELECT) << "Storing selected locale in session key" << sessionKey;
            Session::setValue(c, sessionKey, c->locale());
        } else if (storeTo == LangSelect::Cookie) {
            qCDebug(C_LANGSELECT) << "Storing selected locale in cookie with name" << cookieName;
            c->res()->setCookie(QNetworkCookie(cookieName.toLatin1(), c->locale().bcp47Name().toLatin1()));
        } else if (storeTo == LangSelect::Domain) {
            const auto langDomainParts = domainMap.key(c->locale()).split(QLatin1Char('.'), QString::SkipEmptyParts);
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
            redirect = true;
            if (skipMethod) {
                *skipMethod = true;
            }
        }
    }

    if (addContentLanguageHeader && !redirect) {
        c->res()->setHeader(QStringLiteral("Content-Language"), c->locale().bcp47Name());
    }

    return redirect;
}

void LangSelectPrivate::beforePrepareAction(Context *c, bool *skipMethod)
{
    if (*skipMethod) {
        return;
    }

    if (!c->stash(SELECTION_TRIED).isNull()) {
        return;
    }

    detectLocale(c, sourceOrder, skipMethod);

    c->setStash(SELECTION_TRIED, true);
}

void LangSelectPrivate::_q_postFork(Application *app)
{
    lsp = app->plugin<LangSelect *>();
}

#include "moc_langselect.cpp"
