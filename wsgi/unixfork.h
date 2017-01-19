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

class UnixFork : public QObject
{
    Q_OBJECT
public:
    explicit UnixFork(QObject *parent = 0);
    ~UnixFork();

    void createProcess(int process);

    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);
    static void killSignalHandler(int unused);
    static void intSignalHandler(int unused);
    static void chldSignalHandler(int unused);

    static void setGid(const QString &gid);
    static void setUid(const QString &uid);
    static void chownSocket(const QString &filename, const QString &uidGid);

    void handleSigHup();
    void handleSigTerm();
    void handleSigKill();
    void handleSigInt();
    void handleSigChld();

    int setupUnixSignalHandlers();

Q_SIGNALS:
    void forked();

private:
    bool createChild();

    bool m_child = false;
    QVector<qint64> m_childs;
};

#endif // UNIXFORK_H
