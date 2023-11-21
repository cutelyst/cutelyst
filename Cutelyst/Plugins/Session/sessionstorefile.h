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
/**
 * @ingroup plugins-session
 * @headerfile "" <Cutelyst/Plugins/Session/sessionstorefile.h>
 * @brief A session store that stores user sessions in the file system.
 *
 * This session store stores the session data in the file system. The data is stored in files
 * named after the session id below a directory that is created in the
 * @link QDir::tempPath() system’s temporary directory@endlink and named after your application.
 * For example <TT>/tmp/myapplication/session/data</TT>.
 *
 * This is also the default session store that is used by the Session plugin if no session store
 * has been manuall set via Session::setStorage().
 */
class CUTELYST_PLUGIN_SESSION_EXPORT SessionStoreFile : public SessionStore
{
    Q_OBJECT
public:
    /**
     * Constructs a new %SessionStoreFile object with the given @p parent.
     */
    explicit SessionStoreFile(QObject *parent = nullptr);

    /**
     * Destroys the %SessionStoreFile object.
     */
    ~SessionStoreFile();

    /**
     * Reimplemented from SessionStore::getSessionData().
     */
    virtual QVariant getSessionData(Context *c,
                                    const QByteArray &sid,
                                    const QString &key,
                                    const QVariant &defaultValue) final;

    /**
     * Reimplemented from SessionStore::storeSessionData().
     */
    virtual bool storeSessionData(Context *c,
                                  const QByteArray &sid,
                                  const QString &key,
                                  const QVariant &value) final;

    /**
     * Reimplemented from SessionStore::deleteSessionData().
     */
    virtual bool deleteSessionData(Context *c, const QByteArray &sid, const QString &key) final;

    /**
     * Reimplemented from SessionStore::deleteExpiredSessions().
     */
    virtual bool deleteExpiredSessions(Context *c, quint64 expires) final;
};

} // namespace Cutelyst

#endif // SESSIONSTOREFILE_H
