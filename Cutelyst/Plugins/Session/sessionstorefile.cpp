/*
 * Copyright (C) 2015-2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include "sessionstorefile_p.h"

#include <Cutelyst/Context>

#include <QDir>
#include <QFile>
#include <QLockFile>
#include <QDataStream>
#include <QLoggingCategory>
#include <QCoreApplication>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION_FILE, "cutelyst.plugin.sessionfile")

#define SESSION_STORE_FILE "__session_store_file"
#define SESSION_STORE_FILE_DATA "__session_store_file_data"
#define SESSION_STORE_FILE_TRYLOCK_TIMEOUT 3

SessionStoreFile::SessionStoreFile(QObject *parent) : SessionStore(parent)
  , d_ptr(new SessionStoreFilePrivate)
{

}

QVariant SessionStoreFile::getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue)
{
    Q_D(const SessionStoreFile);
    QVariantHash data;
    QVariant session = c->property(SESSION_STORE_FILE_DATA);

    if (!session.isNull()) {
        data = session.toHash();
    } else {
        QFile *file = d->checkSessionFileStorage(c, sid);
        if (!file) {
            return defaultValue;
        }

        QLockFile lock(file->fileName() + QLatin1String(".lock"));
        if (lock.tryLock(SESSION_STORE_FILE_TRYLOCK_TIMEOUT)) {
            file->seek(0);
            QDataStream in(file);
            in >> data;
            c->setProperty(SESSION_STORE_FILE_DATA, data);
            lock.unlock();
        }
    }

    return data.value(key, defaultValue);
}

bool SessionStoreFile::storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value)
{
    Q_D(const SessionStoreFile);

    QFile *file = d->checkSessionFileStorage(c, sid);
    if (!file) {
        return false;
    }

    QLockFile lock(file->fileName() + QLatin1String(".lock"));
    if (lock.tryLock(SESSION_STORE_FILE_TRYLOCK_TIMEOUT)) {
        QDataStream in(file);
        QVariantHash data;
        QVariant session = c->property(SESSION_STORE_FILE_DATA);

        if (!session.isNull()) {
            data = session.toHash();
        } else {
            file->seek(0);
            in >> data;
            in.resetStatus();
        }

        data.insert(key, value);
        c->setProperty(SESSION_STORE_FILE_DATA, data);

        file->seek(0);
        in << data;
        file->flush();
        lock.unlock();

        return !file->error();
    }

    return false;
}

bool SessionStoreFile::deleteSessionData(Context *c, const QString &sid, const QString &key)
{
    Q_D(const SessionStoreFile);
    QFile *file = d->checkSessionFileStorage(c, sid);
    if (!file) {
        return false;
    }

    QLockFile lock(file->fileName() + QLatin1String(".lock"));
    if (lock.tryLock(SESSION_STORE_FILE_TRYLOCK_TIMEOUT)) {
        QDataStream in(file);
        QVariantHash data;
        QVariant session = c->property(SESSION_STORE_FILE_DATA);

        if (!session.isNull()) {
            data = session.toHash();
        } else {
            file->seek(0);
            in >> data;
            in.resetStatus();
        }

        data.remove(key);
        c->setProperty(SESSION_STORE_FILE_DATA, data);

        if (data.isEmpty()) {
            QFile::remove(file->fileName());
            delete file;
            c->setProperty(SESSION_STORE_FILE, QVariant());
            return true;
        }

        file->seek(0);
        in << data;
        file->flush();
        lock.unlock();

        return !file->error();
    }

    return false;
}

bool SessionStoreFile::deleteExpiredSessions(Context *c, quint64 expires)
{
    Q_UNUSED(c)
    Q_UNUSED(expires)
    return true;
}

QFile *Cutelyst::SessionStoreFilePrivate::checkSessionFileStorage(Context *c, const QString &sid) const
{
    const QVariant sessionVariant = c->property(SESSION_STORE_FILE);
    if (!sessionVariant.isNull()) {
        const auto file = sessionVariant.value<QFile*>();
        if (file) {
            return file;
        }
    }

    const static QString root = QDir::tempPath()
            + QLatin1Char('/')
            + QCoreApplication::applicationName()
            + QLatin1String("/session/data");

    auto file = new QFile(root + QLatin1Char('/') + sid, c);
    if (!file->open(QIODevice::ReadWrite)) {
        if (!QDir().mkpath(root)) {
            qCWarning(C_SESSION_FILE) << "Failed to create path for session object" << root;
            return nullptr;
        }

        if (!file->open(QIODevice::ReadWrite)) {
            return nullptr;
        }
    }

    c->setProperty(SESSION_STORE_FILE, QVariant::fromValue(file));
    return file;
}

#include "moc_sessionstorefile.cpp"
