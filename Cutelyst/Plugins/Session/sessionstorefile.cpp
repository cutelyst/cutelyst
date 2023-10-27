/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "sessionstorefile.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QLockFile>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION_FILE, "cutelyst.plugin.sessionfile", QtWarningMsg)

#define SESSION_STORE_FILE_SAVE QStringLiteral("_c_session_store_file_save")
#define SESSION_STORE_FILE_DATA QStringLiteral("_c_session_store_file_data")

static QVariantHash loadSessionData(Context *c, const QByteArray &sid);

SessionStoreFile::SessionStoreFile(QObject *parent)
    : SessionStore(parent)
{
}

SessionStoreFile::~SessionStoreFile()
{
}

QVariant SessionStoreFile::getSessionData(Context *c,
                                          const QByteArray &sid,
                                          const QString &key,
                                          const QVariant &defaultValue)
{
    const QVariantHash data = loadSessionData(c, sid);

    return data.value(key, defaultValue);
}

bool SessionStoreFile::storeSessionData(Context *c,
                                        const QByteArray &sid,
                                        const QString &key,
                                        const QVariant &value)
{
    QVariantHash data = loadSessionData(c, sid);

    data.insert(key, value);
    c->setStash(SESSION_STORE_FILE_DATA, data);
    c->setStash(SESSION_STORE_FILE_SAVE, true);

    return true;
}

bool SessionStoreFile::deleteSessionData(Context *c, const QByteArray &sid, const QString &key)
{
    QVariantHash data = loadSessionData(c, sid);

    data.remove(key);
    c->setStash(SESSION_STORE_FILE_DATA, data);
    c->setStash(SESSION_STORE_FILE_SAVE, true);

    return true;
}

static QString rootPath()
{
    static QString rootPath =
        QDir::tempPath() + u'/' + QCoreApplication::applicationName() + u"/session/data";
    return rootPath;
}

bool SessionStoreFile::deleteExpiredSessions(Context *c, quint64 expires)
{
    Q_UNUSED(c)
    if (!expires) {
        QDir dir(rootPath());
        dir.removeRecursively();
    }
    return true;
}

QVariantHash loadSessionData(Context *c, const QByteArray &sid)
{
    QVariantHash data;
    const QVariant sessionVariant = c->stash(SESSION_STORE_FILE_DATA);
    if (!sessionVariant.isNull()) {
        data = sessionVariant.toHash();
        return data;
    }

    const QString root = rootPath();

    auto file = new QFile(root + u'/' + QString::fromLatin1(sid), c);
    if (!file->open(QIODevice::ReadWrite)) {
        if (!QDir().mkpath(root)) {
            qCWarning(C_SESSION_FILE) << "Failed to create path for session object" << root;
            return data;
        }

        if (!file->open(QIODevice::ReadWrite)) {
            return data;
        }
    }

    // Commit data when Context gets deleted
    QObject::connect(c->app(), &Application::afterDispatch, c, [c, file] {
        if (!c->stash(SESSION_STORE_FILE_SAVE).toBool()) {
            return;
        }

        const QVariantHash data = c->stash(SESSION_STORE_FILE_DATA).toHash();

        if (data.isEmpty()) {
            QFile::remove(file->fileName());
        } else {
            QLockFile lock(file->fileName() + QLatin1String(".lock"));
            if (lock.lock()) {
                QDataStream in(file);

                if (file->pos()) {
                    file->seek(0);
                }

                in << data;

                if (file->pos() < file->size()) {
                    file->resize(file->pos());
                }

                file->flush();
                lock.unlock();
            }
        }
    });

    // Load data
    QLockFile lock(file->fileName() + QLatin1String(".lock"));
    if (lock.lock()) {
        QDataStream in(file);
        in >> data;
        lock.unlock();
    }

    c->setStash(SESSION_STORE_FILE_DATA, data);

    return data;
}

#include "moc_sessionstorefile.cpp"
