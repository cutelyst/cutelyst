/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "htpasswd.h"

#include "common.h"

#include <QFile>
#include <QLoggingCategory>
#include <QTemporaryFile>

using namespace Cutelyst;

StoreHtpasswd::StoreHtpasswd(const QString &name, QObject *parent)
    : AuthenticationStore(parent)
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
            QByteArray line      = file.readLine();
            QByteArrayList parts = line.split(':');
            if (!wrote && parts.size() >= 2 && parts.first() == username.toLatin1()) {
                line  = username.toLatin1() + ':' + user.value(QStringLiteral("password")).toLatin1().replace(':', ',') + '\n';
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
            QByteArray line      = file.readLine();
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
    return findUser(c, {{QStringLiteral("username"), frozenUser.toString()}});
}

#include "moc_htpasswd.cpp"
