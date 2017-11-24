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

#include "memcachedsessionstore_p.h"

#include <Cutelyst/Application>
#include <Cutelyst/Engine>
#include <Cutelyst/Context>
#include <Cutelyst/Plugins/Memcached/Memcached>

#include <QLoggingCategory>
#include <QCoreApplication>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_MEMCACHEDSESSIONSTORE, "cutelyst.plugin.memcachedsessionstore")

#define SESSION_STORE_MEMCD_SAVE "__session_store_memcd_save"
#define SESSION_STORE_MEMCD_DATA "__session_store_memcd_data"

static QVariantHash loadMemcSessionData(Context *c, const QString &sid, const QString &groupKey);

MemcachedSessionStore::MemcachedSessionStore(Cutelyst::Application *app, QObject *parent) :
    SessionStore(parent), d_ptr(new MemcachedSessionStorePrivate)
{
    Q_D(MemcachedSessionStore);
    Q_ASSERT_X(app, "construct MemachedSessionStore", "you have to specifiy a pointer to the Application object");
    const QVariantMap map = app->engine()->config(QStringLiteral("Cutelyst_MemcachedSessionStore_Plugin"));
    d->groupKey = map.value(QStringLiteral("group_key")).toString();
}

MemcachedSessionStore::~MemcachedSessionStore()
{

}

QVariant MemcachedSessionStore::getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue)
{
    QVariant data;
    Q_D(MemcachedSessionStore);
    const QVariantHash hash = loadMemcSessionData(c, sid, d->groupKey);
    data = hash.value(key, defaultValue);
    return data;
}

bool MemcachedSessionStore::storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value)
{
    Q_D(MemcachedSessionStore);
    QVariantHash data = loadMemcSessionData(c, sid, d->groupKey);
    data.insert(key, value);
    c->setProperty(SESSION_STORE_MEMCD_DATA, data);
    c->setProperty(SESSION_STORE_MEMCD_SAVE, true);

    return true;
}

bool MemcachedSessionStore::deleteSessionData(Context *c, const QString &sid, const QString &key)
{
    Q_D(MemcachedSessionStore);
    QVariantHash data = loadMemcSessionData(c, sid, d->groupKey);
    data.remove(key);
    c->setProperty(SESSION_STORE_MEMCD_DATA, data);
    c->setProperty(SESSION_STORE_MEMCD_SAVE, true);

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
    const QVariant sessionVariant = c->property(SESSION_STORE_MEMCD_DATA);
    if (!sessionVariant.isNull()) {
        data = sessionVariant.toHash();
        return data;
    }

    const static QString sessionPrefix = QCoreApplication::applicationName() + QLatin1String("_sess_");
    const QString sessionKey = sessionPrefix + sid;

    QObject::connect(c, &Context::destroyed, [=] () {
        if (!c->property(SESSION_STORE_MEMCD_SAVE).toBool()) {
            return;
        }

        QVariantHash data = c->property(SESSION_STORE_MEMCD_DATA).toHash();

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
           bool ok = false;
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

    c->setProperty(SESSION_STORE_MEMCD_DATA, data);

    return data;
}

#include "moc_memcachedsessionstore.cpp"
