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

using namespace Cutelyst;

StoreHtpasswd::StoreHtpasswd(const QString &file)
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
    QFile file(fileName);
    if (file.open(QFile::ReadWrite | QFile::Text)) {
        QTemporaryFile tmp;
        if (tmp.open()) {
            while (!file.atEnd()) {
                QByteArray line = file.readLine();
                QList<QByteArray> parts = line.split(':');
                if (parts.size() >= 2 && parts.first() == username) {
                    line = username.toLatin1() + ":" + user.value("password").toLatin1();
                }
                tmp.write(line);
            }
            tmp.rename(fileName);
        }
    }
}

Authentication::User StoreHtpasswd::findUser(Context *ctx, const CStringHash &userInfo)
{
    QString username = userInfo.value("username");

    QString fileName = property("_file").toString();
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QList<QByteArray> parts = line.split(':');
            if (parts.size() >= 2 && !parts.first().startsWith('#') && parts.first() == username) {
                Authentication::User ret;
                ret.insert("username", username);
                ret.setId(username);
                ret.insert("password", parts.at(1));
                return ret;
                // TODO maybe support additional fields
            }
        }
    }
    return Authentication::User();
}

QVariant StoreHtpasswd::forSession(Context *ctx, const Authentication::User &user)
{
    return user.id();
}

Authentication::User StoreHtpasswd::fromSession(Context *ctx, const QVariant &frozenUser)
{
    CStringHash userInfo;
    userInfo[QStringLiteral("id")] = frozenUser.toString();
    return findUser(ctx, userInfo);
}
