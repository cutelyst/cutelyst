/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "sessionstorememcached_p.h"

#include <Cutelyst/Context>

#include <QLoggingCategory>
#include <QDataStream>
#include <QCoreApplication>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION_MEMCACHED, "cutelyst.plugin.sessionmemcached")

#define SESSION_STORE_MEMCD_SAVE "__session_store_memcd_save"
#define SESSION_STORE_MEMCD_DATA "__session_store_memcd_data"

static QVariantHash loadMemcSessionData(Context *c, const QString &sid, const std::__cxx11::string &config);

SessionStoreMemcached::SessionStoreMemcached(QObject *parent) :
    SessionStore(parent), d_ptr(new SessionStoreMemcachedPrivate)
{

}

SessionStoreMemcached::SessionStoreMemcached(const QString &config, QObject *parent) :
    SessionStore(parent), d_ptr(new SessionStoreMemcachedPrivate(config))
{

}

SessionStoreMemcached::~SessionStoreMemcached()
{
    delete d_ptr;
}

QVariant SessionStoreMemcached::getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue)
{
    const QVariantHash data = loadMemcSessionData(c, sid, d_ptr->config);

    return data.value(key, defaultValue);
}

bool SessionStoreMemcached::storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value)
{
    QVariantHash data = loadMemcSessionData(c, sid, d_ptr->config);

    data.insert(key, value);
    c->setProperty(SESSION_STORE_MEMCD_DATA, data);
    c->setProperty(SESSION_STORE_MEMCD_SAVE, true);

    return true;
}

bool SessionStoreMemcached::deleteSessionData(Context *c, const QString &sid, const QString &key)
{
    QVariantHash data = loadMemcSessionData(c, sid, d_ptr->config);

    data.remove(key);
    c->setProperty(SESSION_STORE_MEMCD_DATA, data);
    c->setProperty(SESSION_STORE_MEMCD_SAVE, true);

    return true;
}

bool SessionStoreMemcached::deleteExpiredSessions(Context *c, quint64 expires)
{
    Q_UNUSED(c)
    Q_UNUSED(expires)
    return true;
}

QVariantHash loadMemcSessionData(Context *c, const QString &sid, const std::string &config)
{
    QVariantHash data;
    const QVariant sessionVariant = c->property(SESSION_STORE_MEMCD_DATA);
    if (!sessionVariant.isNull()) {
        data = sessionVariant.toHash();
        return data;
    }

    const static QString sessionPrefix = QCoreApplication::applicationName() + QLatin1String("_sess_");
    const QString sessionKey = sessionPrefix + sid;

    static memcache::Memcache memc(config);

    QObject::connect(c, &Context::destroyed, [=] () {
        if (!c->property(SESSION_STORE_MEMCD_SAVE).toBool()) {
            return;
        }

        QVariantHash data = c->property(SESSION_STORE_MEMCD_DATA).toHash();

        if (data.isEmpty()) {
            if (!memc.remove(sessionKey.toStdString(), QDateTime::currentDateTimeUtc().toTime_t())) {
                std::string errorString;
                memc.error(errorString);
                qCWarning(C_SESSION_MEMCACHED) << "Failed to remove session from Memcached:" << QString::fromStdString(errorString);
            }
        } else {
            QByteArray storeData;
            QDataStream out(&storeData, QIODevice::WriteOnly);
            out << data;

            if (!memc.set(sessionKey.toStdString(),
                          storeData.data(),
                          storeData.size(),
                          data.value(QStringLiteral("expires")).toUInt(),
                          (uint32_t)0))
            {
                std::string errorString;
                memc.error(errorString);
                qCWarning(C_SESSION_MEMCACHED) << "Failed to store session to Memcached:" << QString::fromStdString(errorString);
            }
        }
    });

    std::vector<char> storedData;
    if (memc.get(sessionKey.toStdString(), storedData)) {
        if (!storedData.empty()) {
            QByteArray storedArray;
            for (auto it = storedData.cbegin(); it != storedData.cend(); ++it) {
                storedArray.append(*it);
            }
            if (!storedArray.isNull()) {
                QDataStream in(&storedArray, QIODevice::ReadOnly);
                in >> data;
            }
        }
    }

    c->setProperty(SESSION_STORE_MEMCD_DATA, data);

    return data;
}
