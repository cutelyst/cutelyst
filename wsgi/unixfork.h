/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef UNIXFORK_H
#define UNIXFORK_H

#include <QObject>
#include <QHash>
#include <QVector>

#include "abstractfork.h"

typedef struct {
    bool null = true;
    int id;
    int restart = 0;
    int respawn = 0;
} Worker;

namespace CWSGI {
class WSGI;
}

class QTimer;
class QSocketNotifier;
class UnixFork : public AbstractFork
{
    Q_OBJECT
public:
    explicit UnixFork(int process, int threads, QObject *parent = 0);
    ~UnixFork();

    virtual bool continueMaster(int *exit = 0) override;

    virtual int exec(bool lazy, bool master) override;

    virtual void restart() override;

    int internalExec();

    bool createProcess(bool respawn);
    void decreaseWorkerRespawn();

    virtual void killChild() override;
    void killChild(qint64 pid);

    virtual void terminateChild() override;
    void terminateChild(qint64 pid);

    static void stopWSGI(const QString &pidfile);

    static bool setUmask(const QString &valueStr);
    static void setGidUid(const QString &gid, const QString &uid, bool noInitgroups);
    static void chownSocket(const QString &filename, const QString &uidGid);

    static int idealProcessCount();
    static int idealThreadCount();

    void handleSigHup();
    void handleSigTerm();
    void handleSigInt();
    void handleSigChld();

    static void setSched(CWSGI::WSGI *wsgi, int workerId, int workerCore);

private:
    int setupUnixSignalHandlers();
    void setupSocketPair(bool closeSignalsFD);
    bool createChild(const Worker &worker, bool respawn);
    static void signalHandler(int signal);
    void setupCheckChildTimer();
    void postFork(int workerId);

    QHash<qint64, Worker> m_childs;
    QVector<Worker> m_recreateWorker;
    QSocketNotifier *m_signalNotifier = nullptr;
    QTimer *m_checkChildRestart = nullptr;
    int m_threads;
    int m_processes;
    bool m_child = false;
    bool m_terminating = false;
};

#endif // UNIXFORK_H
