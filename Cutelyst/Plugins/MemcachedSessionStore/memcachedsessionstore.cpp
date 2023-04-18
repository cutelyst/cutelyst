/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "memcachedsessionstore_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Engine>
#include <Cutelyst/Plugins/Memcached/Memcached>

#include <QCoreApplication>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_MEMCACHEDSESSIONSTORE, "cutelyst.plugin.memcachedsessionstore", QtWarningMsg)

#define SESSION_STORE_MEMCD_SAVE QStringLiteral("_c_session_store_memcd_save")
#define SESSION_STORE_MEMCD_DATA QStringLiteral("_c_session_store_memcd_data")

static QVariantHash loadMemcSessionData(Context *c, const QString &sid, const QString &groupKey);

MemcachedSessionStore::MemcachedSessionStore(Cutelyst::Application *app, QObject *parent)
    : SessionStore(parent)
    , d_ptr(new MemcachedSessionStorePrivate)
{
    Q_D(MemcachedSessionStore);
    Q_ASSERT_X(app, "construct MemachedSessionStore", "you have to specifiy a pointer to the Application object");
    const QVariantMap map = app->engine()->config(QStringLiteral("Cutelyst_MemcachedSessionStore_Plugin"));
    d->groupKey           = map.value(QStringLiteral("group_key")).toString();
}

MemcachedSessionStore::~MemcachedSessionStore()
{
}

QVariant MemcachedSessionStore::getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue)
{
    QVariant data;
    Q_D(MemcachedSessionStore);
    const QVariantHash hash = loadMemcSessionData(c, sid, d->groupKey);
    data                    = hash.value(key, defaultValue);
    return data;
}

bool MemcachedSessionStore::storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value)
{
    Q_D(MemcachedSessionStore);
    QVariantHash data = loadMemcSessionData(c, sid, d->groupKey);
    data.insert(key, value);
    c->setStash(SESSION_STORE_MEMCD_DATA, data);
    c->setStash(SESSION_STORE_MEMCD_SAVE, true);

    return true;
}

bool MemcachedSessionStore::deleteSessionData(Context *c, const QString &sid, const QString &key)
{
    Q_D(MemcachedSessionStore);
    QVariantHash data = loadMemcSessionData(c, sid, d->groupKey);
    data.remove(key);
    c->setStash(SESSION_STORE_MEMCD_DATA, data);
    c->setStash(SESSION_STORE_MEMCD_SAVE, true);

    return true;
}

bool MemcachedSessionStore::deleteExpiredSessions(Context *c, quint64 expires)
{
    Q_UNUSED(c)
    Q_UNUSED(expires)

    return true;
}

void MemcachedSessionStore::setGroupKey(const QString &groupKey)
{
    Q_D(MemcachedSessionStore);
    d->groupKey = groupKey;
}

QVariantHash loadMemcSessionData(Context *c, const QString &sid, const QString &groupKey)
{
    QVariantHash data;
    const QVariant sessionVariant = c->stash(SESSION_STORE_MEMCD_DATA);
    if (!sessionVariant.isNull()) {
        data = sessionVariant.toHash();
        return data;
    }

    const static QString sessionPrefix = QCoreApplication::applicationName() + QLatin1String("_sess_");
    const QString sessionKey           = sessionPrefix + sid;

    QObject::connect(c->app(), &Application::afterDispatch, c, [=]() {
        if (!c->stash(SESSION_STORE_MEMCD_SAVE).toBool()) {
            return;
        }

        const QVariantHash data = c->stash(SESSION_STORE_MEMCD_DATA).toHash();

        if (data.isEmpty()) {
            bool ok = false;
            if (groupKey.isEmpty()) {
                ok = Memcached::remove(sessionKey);
            } else {
                ok = Memcached::removeByKey(groupKey, sessionKey);
            }
            if (!ok) {
                qCWarning(C_MEMCACHEDSESSIONSTORE, "Failed to remove session from Memcached.");
            }
        } else {
            bool ok              = false;
            const time_t expires = data.value(QStringLiteral("expires")).value<time_t>();
            if (groupKey.isEmpty()) {
                ok = Memcached::set(sessionKey, data, expires);
            } else {
                ok = Memcached::setByKey(groupKey, sessionKey, data, expires);
            }
            if (!ok) {
                qCWarning(C_MEMCACHEDSESSIONSTORE, "Failed to store session to Memcached.");
            }
        }
    });

    if (groupKey.isEmpty()) {
        data = Memcached::get<QVariantHash>(sessionKey);
    } else {
        data = Memcached::getByKey<QVariantHash>(groupKey, sessionKey);
    }

    c->setStash(SESSION_STORE_MEMCD_DATA, data);

    return data;
}

#include "moc_memcachedsessionstore.cpp"
