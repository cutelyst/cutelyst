/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
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

#include "htpasswd.h"

#include <QFile>
#include <QTemporaryFile>
#include <QLoggingCategory>
#include <QStringBuilder>

#include "common.h"

using namespace Cutelyst;

StoreHtpasswd::StoreHtpasswd(const QString &file, QObject *parent) : AuthenticationStore(parent)
{
    setProperty("_file", file);
}

StoreHtpasswd::~StoreHtpasswd()
{

}

void StoreHtpasswd::addUser(const CStringHash &user)
{
    QString username = user.value("username");

    QString fileName = property("_file").toString();
    QTemporaryFile tmp(fileName % QLatin1String("-XXXXXXX"));
    tmp.setAutoRemove(false); // sort of a backup
    if (!tmp.open()) {
        qCWarning(CUTELYST_UTILS_AUTH) << "Failed to open temporary file for writting";
        return;
    }

    bool wrote = false;
    QFile file(fileName);
    if (file.exists() && file.open(QFile::ReadWrite | QFile::Text)) {
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QList<QByteArray> parts = line.split(':');
            if (!wrote && parts.size() >= 2 && parts.first() == username) {
                line = username.toLatin1() + ':' + user.value("password").toLatin1() + '\n';
                wrote = true;
            }
            tmp.write(line);
        }
        file.close();
    }

    if (!wrote) {
        QByteArray line = username.toLatin1() + ':' + user.value("password").toLatin1() + '\n';
        tmp.write(line);
    }

    if (file.exists() && !file.remove()) {
        qCWarning(CUTELYST_UTILS_AUTH) << "Failed to remove auth file for replacement";
        return;
    }

    if (!tmp.rename(fileName)) {
        qCWarning(CUTELYST_UTILS_AUTH) << "Failed to rename temporary file";
    }
}

AuthenticationUser StoreHtpasswd::findUser(Context *ctx, const CStringHash &userInfo)
{
    QString username = userInfo.value("username");

    QString fileName = property("_file").toString();
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QList<QByteArray> parts = line.trimmed().split(':');
            if (parts.size() >= 2 && !parts.first().startsWith('#') && parts.first() == username) {
                AuthenticationUser ret;
                ret.insert("username", username);
                ret.setId(username);
                ret.insert("password", parts.at(1));
                return ret;
                // TODO maybe support additional fields
            }
        }
    }
    return AuthenticationUser();
}

QVariant StoreHtpasswd::forSession(Context *ctx, const AuthenticationUser &user)
{
    return user.id();
}

AuthenticationUser StoreHtpasswd::fromSession(Context *c, const QVariant &frozenUser)
{
    return findUser(c, {
                        {QStringLiteral("username"), frozenUser.toString()}
                    });
}
