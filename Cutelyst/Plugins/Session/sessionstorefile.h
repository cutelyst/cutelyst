/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef SESSIONSTOREFILE_H
#define SESSIONSTOREFILE_H

#include <Cutelyst/Plugins/Session/session.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class SessionStoreFilePrivate;
class CUTELYST_PLUGIN_SESSION_EXPORT SessionStoreFile : public SessionStore
{
    Q_OBJECT
public:
    /**
     * Constructs a new session store file object with the given parent.
     */
    explicit SessionStoreFile(QObject *parent = nullptr);
    ~SessionStoreFile();

    /**
     * Reimplemented from SessionStore::getSessionData().
     */
    virtual QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue) final;

    /**
     * Reimplemented from SessionStore::storeSessionData().
     */
    virtual bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) final;

    /**
     * Reimplemented from SessionStore::deleteSessionData().
     */
    virtual bool deleteSessionData(Context *c, const QString &sid, const QString &key) final;

    /**
     * Reimplemented from SessionStore::deleteExpiredSessions().
     */
    virtual bool deleteExpiredSessions(Context *c, quint64 expires) final;
};

} // namespace Cutelyst

#endif // SESSIONSTOREFILE_H
