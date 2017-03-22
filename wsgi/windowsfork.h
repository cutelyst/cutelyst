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
#ifndef WINDOWSFORK_H
#define WINDOWSFORK_H

#include <QObject>
#include <QProcess>

#include "abstractfork.h"

class QTimer;
class WindowsFork : public AbstractFork
{
    Q_OBJECT
public:
    explicit WindowsFork(QObject *parent = 0);

    virtual bool continueMaster(int *exit = 0) override;

    virtual int exec(bool lazy, bool master) override;

    virtual void killChild() override;
    virtual void terminateChild() override;

    virtual void restart() override;

private:
    void startChild();
    void childFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void restartTerminate();

    QProcess *m_masterChildProcess = nullptr;
    QTimer *m_materChildRestartTimer = nullptr;
    int m_autoReloadCount = 0;
};

#endif // WINDOWSFORK_H
