/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
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
    virtual bool continueMaster(int *exit = nullptr) = 0;

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

protected:
    void fileChanged(const QString &path);
    void directoryChanged(const QString &path);

private:
    QStringList m_touchReloadPaths;
    QFileSystemWatcher *m_touchReloadWatcher = nullptr;
    QTimer *m_restartTimer                   = nullptr;
};

#endif // ABSTRACTFORK_H
