/*
 * SPDX-FileCopyrightText: (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef WINDOWSFORK_H
#define WINDOWSFORK_H

#include "abstractfork.h"

#include <QObject>
#include <QProcess>

class QTimer;
class WindowsFork : public AbstractFork
{
    Q_OBJECT
public:
    explicit WindowsFork(QObject *parent = nullptr);

    virtual bool continueMaster(int *exit = nullptr) override;

    virtual int exec(bool lazy, bool master) override;

    virtual void killChild() override;
    virtual void terminateChild() override;

    virtual void restart() override;

private:
    void startChild();
    void childFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void restartTerminate();

    QProcess *m_masterChildProcess   = nullptr;
    QTimer *m_materChildRestartTimer = nullptr;
    int m_autoReloadCount            = 0;
};

#endif // WINDOWSFORK_H
