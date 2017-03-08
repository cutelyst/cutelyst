/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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

class QSocketNotifier;
class UnixFork : public QObject
{
    Q_OBJECT
public:
    explicit UnixFork(int process, int threads, QObject *parent = 0);
    ~UnixFork();

    int exec();
    bool createProcess(bool respawn);

    void killChild();
    void terminateChild();

    static void stopWSGI(const QString &pidfile);

    static bool setUmask(const QString &valueStr);
    static void setGid(const QString &gid);
    static void setUid(const QString &uid);
    static void chownSocket(const QString &filename, const QString &uidGid);

    void handleSigHup();
    void handleSigTerm();
    void handleSigInt();
    void handleSigChld();

Q_SIGNALS:
    void forked();
    void shutdown();

private:
    int setupUnixSignalHandlers();
    void setupSocketPair(bool closeSignalsFD);
    bool createChild(int worker, bool respawn);
    static void signalHandler(int signal);

    QHash<qint64, int> m_childs;
    QVector<int> m_recreateWorker;
    QSocketNotifier *m_signalNotifier = nullptr;
    int m_threads;
    int m_processes;
    bool m_child = false;
    bool m_terminating = false;
};

#endif // UNIXFORK_H
