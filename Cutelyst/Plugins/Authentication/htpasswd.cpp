/*
 * Copyright (C) 2014-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include "htpasswd.h"

#include <QFile>
#include <QTemporaryFile>
#include <QLoggingCategory>

#include "common.h"

using namespace Cutelyst;

StoreHtpasswd::StoreHtpasswd(const QString &name, QObject *parent) : AuthenticationStore(parent)
  , m_filename(name)
{

}

StoreHtpasswd::~StoreHtpasswd()
{

}

void StoreHtpasswd::addUser(const ParamsMultiMap &user)
{
    const QString username = user.value(QStringLiteral("username"));

    QTemporaryFile tmp(m_filename + QLatin1String("-XXXXXXX"));
    tmp.setAutoRemove(false); // sort of a backup
    if (!tmp.open()) {
        qCWarning(CUTELYST_UTILS_AUTH) << "Failed to open temporary file for writing";
        return;
    }

    bool wrote = false;
    QFile file(m_filename);
    if (file.exists() && file.open(QFile::ReadWrite | QFile::Text)) {
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QByteArrayList parts = line.split(':');
            if (!wrote && parts.size() >= 2 && parts.first() == username.toLatin1()) {
                line = username.toLatin1() + ':' + user.value(QStringLiteral("password")).toLatin1().replace(':', ',') + '\n';
                wrote = true;
            }
            tmp.write(line);
        }
        file.close();
    }

    if (!wrote) {
        QByteArray line = username.toLatin1() + ':' + user.value(QStringLiteral("password")).toLatin1().replace(':', ',') + '\n';
        tmp.write(line);
    }

    if (file.exists() && !file.remove()) {
        qCWarning(CUTELYST_UTILS_AUTH) << "Failed to remove auth file for replacement";
        return;
    }

    if (!tmp.rename(m_filename)) {
        qCWarning(CUTELYST_UTILS_AUTH) << "Failed to rename temporary file";
    }
}

AuthenticationUser StoreHtpasswd::findUser(Context *c, const ParamsMultiMap &userInfo)
{
    Q_UNUSED(c);
    AuthenticationUser ret;
    const QString username = userInfo.value(QStringLiteral("username"));

    QFile file(m_filename);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QByteArrayList parts = line.trimmed().split(':');
            if (parts.size() >= 2 && !parts.first().startsWith('#') && parts.first() == username.toLatin1()) {
                ret.insert(QStringLiteral("username"), username);
                ret.setId(username);
                QByteArray password = parts.at(1);
                ret.insert(QStringLiteral("password"), QString::fromLatin1(password.replace(',', ':')));
                break;
            }
        }
    }
    return ret;
}

QVariant StoreHtpasswd::forSession(Context *c, const AuthenticationUser &user)
{
    Q_UNUSED(c);
    return user.id();
}

AuthenticationUser StoreHtpasswd::fromSession(Context *c, const QVariant &frozenUser)
{
    return findUser(c, {
                        {QStringLiteral("username"), frozenUser.toString()}
                    });
}

#include "moc_htpasswd.cpp"
