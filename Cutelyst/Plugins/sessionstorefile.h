/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef SESSIONSTOREFILE_H
#define SESSIONSTOREFILE_H

#include <QObject>
#include <Cutelyst/Plugins/session.h>

namespace Cutelyst {

class SessionStoreFilePrivate;
class SessionStoreFile : public SessionStore
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(SessionStoreFile)
public:
    explicit SessionStoreFile(QObject *parent = 0);

    QVariant getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue) Q_DECL_FINAL;
    bool storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value) Q_DECL_FINAL;
    bool deleteSessionData(Context *c, const QString &sid, const QString &key) Q_DECL_FINAL;
    bool deleteExpiredSessions(Context *c, quint64 expires) Q_DECL_FINAL;

protected:
    SessionStoreFilePrivate *d_ptr;
};

}

#endif // SESSIONSTOREFILE_H
