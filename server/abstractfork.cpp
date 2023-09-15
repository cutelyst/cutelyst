/*
 * SPDX-FileCopyrightText: (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "abstractfork.h"

#include <iostream>

#include <QFileSystemWatcher>
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(WSGI_FORK, "wsgi.fork", QtWarningMsg)

AbstractFork::AbstractFork(QObject *parent)
    : QObject(parent)
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
        connect(m_touchReloadWatcher,
                &QFileSystemWatcher::fileChanged,
                this,
                &AbstractFork::fileChanged);
        connect(m_touchReloadWatcher,
                &QFileSystemWatcher::directoryChanged,
                this,
                &AbstractFork::directoryChanged);
        const QStringList ret = m_touchReloadWatcher->addPaths(m_touchReloadPaths);
        if (!ret.empty()) {
            std::cerr << "Failed setup file watcher" << std::endl;
            qCCritical(WSGI_FORK) << "unwatched files" << ret;
            exit(1);
        }

        m_restartTimer = new QTimer(this);
        connect(m_restartTimer, &QTimer::timeout, this, &AbstractFork::restart);
        m_restartTimer->setInterval(std::chrono::seconds{1});
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
