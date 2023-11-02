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

const QString MemcachedSessionStorePrivate::stashKeyMemcdSave{u"_c_session_store_memcd_save"_qs};
const QString MemcachedSessionStorePrivate::stashKeyMemcdData{u"_c_session_store_memcd_data"_qs};

static QVariantHash
    loadMemcSessionData(Context *c, const QByteArray &sid, const QByteArray &groupKey);

MemcachedSessionStore::MemcachedSessionStore(Cutelyst::Application *app, QObject *parent)
    : SessionStore(parent)
    , d_ptr(new MemcachedSessionStorePrivate)
{
    Q_D(MemcachedSessionStore);
    Q_ASSERT_X(app,
               "construct MemachedSessionStore",
               "you have to specifiy a pointer to the Application object");
    const QVariantMap map = app->engine()->config(u"Cutelyst_MemcachedSessionStore_Plugin"_qs);
    d->groupKey           = map.value(u"group_key"_qs).toString().toLatin1();
}

MemcachedSessionStore::~MemcachedSessionStore() = default;

QVariant MemcachedSessionStore::getSessionData(Context *c,
                                               const QByteArray &sid,
                                               const QString &key,
                                               const QVariant &defaultValue)
{
    QVariant data;
    Q_D(MemcachedSessionStore);
    const QVariantHash hash = loadMemcSessionData(c, sid, d->groupKey);
    data                    = hash.value(key, defaultValue);
    return data;
}

bool MemcachedSessionStore::storeSessionData(Context *c,
                                             const QByteArray &sid,
                                             const QString &key,
                                             const QVariant &value)
{
    Q_D(MemcachedSessionStore);
    QVariantHash data = loadMemcSessionData(c, sid, d->groupKey);
    data.insert(key, value);
    c->setStash(MemcachedSessionStorePrivate::stashKeyMemcdData, data);
    c->setStash(MemcachedSessionStorePrivate::stashKeyMemcdSave, true);

    return true;
}

bool MemcachedSessionStore::deleteSessionData(Context *c, const QByteArray &sid, const QString &key)
{
    Q_D(MemcachedSessionStore);
    QVariantHash data = loadMemcSessionData(c, sid, d->groupKey);
    data.remove(key);
    c->setStash(MemcachedSessionStorePrivate::stashKeyMemcdData, data);
    c->setStash(MemcachedSessionStorePrivate::stashKeyMemcdSave, true);

    return true;
}

bool MemcachedSessionStore::deleteExpiredSessions(Context *c, quint64 expires)
{
    Q_UNUSED(c)
    Q_UNUSED(expires)

    return true;
}

void MemcachedSessionStore::setGroupKey(const QByteArray &groupKey)
{
    Q_D(MemcachedSessionStore);
    d->groupKey = groupKey;
}

QVariantHash loadMemcSessionData(Context *c, const QByteArray &sid, const QByteArray &groupKey)
{
    QVariantHash data;
    const QVariant sessionVariant = c->stash(MemcachedSessionStorePrivate::stashKeyMemcdData);
    if (!sessionVariant.isNull()) {
        data = sessionVariant.toHash();
        return data;
    }

    const static QByteArray sessionPrefix =
        QCoreApplication::applicationName().toLatin1() + "_sess_";
    const QByteArray sessionKey = sessionPrefix + sid;

    QObject::connect(c->app(), &Application::afterDispatch, c, [=]() {
        if (!c->stash(MemcachedSessionStorePrivate::stashKeyMemcdSave).toBool()) {
            return;
        }

        const QVariantHash data =
            c->stash(MemcachedSessionStorePrivate::stashKeyMemcdData).toHash();

        if (data.isEmpty()) {
            bool ok = false;
            if (groupKey.isEmpty()) {
                ok = Memcached::remove(sessionKey);
            } else {
                ok = Memcached::removeByKey(groupKey, sessionKey);
            }
            if (!ok) {
                qCWarning(C_MEMCACHEDSESSIONSTORE) << "Failed to remove session from Memcached.";
            }
        } else {
            bool ok            = false;
            const auto expires = data.value(u"expires"_qs).value<time_t>();
            if (groupKey.isEmpty()) {
                ok = Memcached::set(sessionKey, data, expires);
            } else {
                ok = Memcached::setByKey(groupKey, sessionKey, data, expires);
            }
            if (!ok) {
                qCWarning(C_MEMCACHEDSESSIONSTORE) << "Failed to store session to Memcached.";
            }
        }
    });

    if (groupKey.isEmpty()) {
        data = Memcached::get<QVariantHash>(sessionKey);
    } else {
        data = Memcached::getByKey<QVariantHash>(groupKey, sessionKey);
    }

    c->setStash(MemcachedSessionStorePrivate::stashKeyMemcdData, data);

    return data;
}

#include "moc_memcachedsessionstore.cpp"
