/*
 * Copyright (C) 2013 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CSESSION_H
#define CSESSION_H

#include "plugin.h"

#include <QObject>
#include <QVariant>

namespace Cutelyst {

class Context;
namespace Plugin {

class Session : public AbstractPlugin
{
    Q_OBJECT
public:
    explicit Session(QObject *parent = 0);

    bool setup(Context *ctx);

    QVariant value(const QString &key, const QVariant &defaultValue = QVariant());
    void setValue(const QString &key, const QVariant &value);
    void deleteValue(const QString &keys);

    bool isValid();

protected:
    /**
     * This method is used to call the storage class and retrieve
     * the session, the default implementation does that by using a file
     */
    virtual QVariantHash retrieveSession(const QString &sessionId) const;

    /**
     * This methos is used to call the storage class and persist
     * the session data, the default implementation does that by using a file
     */
    virtual void persistSession(const QString &sessionId, const QVariant &data) const;

private:
    void saveSession();
    QVariant loadSession();
    QString generateSessionId() const;
    QString getSessionId() const;
    QString filePath(const QString &sessionId) const;

    Context *m_ctx;
    QString m_sessionName;
};

}

}

#endif // CSESSION_H
