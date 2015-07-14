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
#include "sessionstorefile_p.h"

#include <context.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QSettings>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QStringBuilder>
#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_SESSION_FILE, "cutelyst.plugin.sessionfile")

#define SESSION_STORE_FILE "__session_store_file"

SessionStoreFile::SessionStoreFile(QObject *parent) : SessionStore(parent)
  , d_ptr(new SessionStoreFilePrivate)
{

}

QVariant SessionStoreFile::getSessionData(Context *c, const QString &sid, const QString &key, const QVariant &defaultValue = QVariant())
{
    Q_D(const SessionStoreFile);
    QSettings *settings = d->checkSessionFileStorage(c, sid);
    return settings->value(key, defaultValue);
}

bool SessionStoreFile::storeSessionData(Context *c, const QString &sid, const QString &key, const QVariant &value)
{
    Q_D(const SessionStoreFile);
    QSettings *settings = d->checkSessionFileStorage(c, sid);
    settings->setValue(key, value);
    settings->sync();
    return true;
}

bool SessionStoreFile::deleteSessionData(Context *c, const QString &sid, const QString &key)
{
    Q_D(const SessionStoreFile);
    QSettings *settings = d->checkSessionFileStorage(c, sid);
    settings->remove(key);
    if (settings->allKeys().isEmpty()) {
        QFile::remove(settings->fileName());
        delete settings;
        c->setProperty(SESSION_STORE_FILE, QVariant());
    }
    return true;
}

QSettings *Cutelyst::SessionStoreFilePrivate::checkSessionFileStorage(Context *c, const QString &sid) const
{
    QVariant sessionVariant = c->property(SESSION_STORE_FILE);
    if (!sessionVariant.isNull()) {
        QSettings *settings = sessionVariant.value<QSettings*>();
        if (settings) {
            return settings;
        }
    }

    QString root = QDir::tempPath()
            % QDir::separator() % QCoreApplication::applicationName()
            % QDir::separator() % QStringLiteral("session")
            % QDir::separator() % QStringLiteral("data");
    QDir dir;
    if (!dir.mkpath(root)) {
        qCWarning(C_SESSION_FILE) << "Failed to create path for session object" << root;
    }

    QSettings *settings = new QSettings(root % QDir::separator() % sid, QSettings::IniFormat, c);
    settings->beginGroup(QStringLiteral("Session"));
    c->setProperty(SESSION_STORE_FILE, QVariant::fromValue(settings));
    return settings;
}
