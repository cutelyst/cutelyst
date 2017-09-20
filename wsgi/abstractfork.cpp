/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#include "abstractfork.h"

#include <QFileSystemWatcher>
#include <QLoggingCategory>
#include <QTimer>

#include <iostream>

Q_LOGGING_CATEGORY(WSGI_FORK, "wsgi.fork")

AbstractFork::AbstractFork(QObject *parent) : QObject(parent)
{

}

void AbstractFork::setTouchReload(const QStringList &paths)
{
    m_touchReloadPaths = paths;
}

void AbstractFork::installTouchReload()
{
    if (!m_touchReloadPaths.isEmpty() && !m_touchReloadWatcher) {
        m_touchReloadWatcher = new QFileSystemWatcher(this);
        connect(m_touchReloadWatcher, &QFileSystemWatcher::fileChanged, this, &AbstractFork::fileChanged);
        connect(m_touchReloadWatcher, &QFileSystemWatcher::directoryChanged, this, &AbstractFork::directoryChanged);
        const QStringList ret = m_touchReloadWatcher->addPaths(m_touchReloadPaths);
        if (!ret.empty()) {
            std::cerr << "Failed setup file watcher" << std::endl;
            qCCritical(WSGI_FORK) << "unwatched files" << ret;
            exit(1);
        }

        m_restartTimer = new QTimer(this);
        connect(m_restartTimer, &QTimer::timeout, this, &AbstractFork::restart);
        m_restartTimer->setInterval(1 * 1000);
        m_restartTimer->setSingleShot(true);
    }
}

void AbstractFork::removeTouchReload()
{
    delete m_touchReloadWatcher;
    m_touchReloadWatcher = nullptr;

    delete m_restartTimer;
}

void AbstractFork::fileChanged(const QString &path)
{
    std::cout << "File changed restarting... " << qPrintable(path) << std::endl;
    m_restartTimer->start();
}

void AbstractFork::directoryChanged(const QString &path)
{
    std::cout << "Directory changed restarting... " << qPrintable(path) << std::endl;
    m_restartTimer->start();
}

#include "moc_abstractfork.cpp"
