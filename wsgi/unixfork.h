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
#include <QVector>

class QSocketNotifier;
class UnixFork : public QObject
{
    Q_OBJECT
public:
    explicit UnixFork(QObject *parent = 0);
    ~UnixFork();

    void createProcess(int process, int threads);

    void killChild();
    void terminateChild();

    static void setGid(const QString &gid);
    static void setUid(const QString &uid);
    static void chownSocket(const QString &filename, const QString &uidGid);

    void handleSigHup();
    void handleSigTerm();
    void handleSigInt();
    void handleSigChld();


Q_SIGNALS:
    void forked();

private:
    int setupUnixSignalHandlers();
    void setupSocketPair(bool closeSignalsFD);
    bool createChild();
    static void signalHandler(int signal);

    QVector<qint64> m_childs;
    QSocketNotifier *m_signalNotifier = nullptr;
    int m_threads = 0;
    bool m_child = false;
    bool m_terminating = false;
};

#endif // UNIXFORK_H
