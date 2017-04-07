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
#ifndef ABSTRACTFORK_H
#define ABSTRACTFORK_H

#include <QObject>

class QTimer;
class QFileSystemWatcher;
class AbstractFork : public QObject
{
    Q_OBJECT
public:
    explicit AbstractFork(QObject *parent = nullptr);

    /**
     * When in MASTER mode this method is called,
     * it should return true if WSGI::exec() should
     * continue setting up the listening socked or exit
     * with exit code
     */
    virtual bool continueMaster(int *exit = 0) = 0;

    /**
     * Start event loop, it's useful for doing
     * trickery when forking
     */
    virtual int exec(bool lazy, bool master) = 0;

    /**
     * Called when child process should be nicely killed
     */
    virtual void killChild() = 0;

    /**
     * Called when child process should be brutaly killed
     */
    virtual void terminateChild() = 0;

    /**
     * Called to restart process when any of the watched paths changes
     */
    virtual void restart() = 0;

    void setTouchReload(const QStringList &paths);

    void installTouchReload();
    void removeTouchReload();

Q_SIGNALS:
    void forked(int workerId);
    void shutdown();
    void setupApplication();

protected:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    QStringList m_touchReloadPaths;
    QFileSystemWatcher *m_touchReloadWatcher = nullptr;
    QTimer *m_restartTimer = nullptr;
};

#endif // ABSTRACTFORK_H
