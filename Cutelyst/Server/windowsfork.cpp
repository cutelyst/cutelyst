/*
 * SPDX-FileCopyrightText: (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "windowsfork.h"

#include <windows.h>

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QTimer>

Q_LOGGING_CATEGORY(C_SERVER_WIN, "cutelyst.server.windows", QtWarningMsg)

using namespace std::chrono_literals;
using namespace Qt::StringLiterals;

WindowsFork::WindowsFork(QObject *parent)
    : AbstractFork(parent)
{
}

bool WindowsFork::continueMaster(int *exit)
{
    installTouchReload();

    m_masterChildProcess = new QProcess(this);
    connect(m_masterChildProcess, &QProcess::finished, this, &WindowsFork::childFinished);

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert(u"CUTELYST_WSGI_IGNORE_MASTER"_s, u"1"_s);
    m_masterChildProcess->setProcessEnvironment(env);

    m_masterChildProcess->setProcessChannelMode(QProcess::ForwardedChannels);

    m_masterChildProcess->setProgram(QCoreApplication::applicationFilePath());

    m_masterChildProcess->setArguments(QCoreApplication::arguments());

    startChild();

    int ret = qApp->exec();
    if (exit) {
        *exit = ret;
    }
    return false;
}

int WindowsFork::exec(bool lazy, bool master)
{
    Q_UNUSED(lazy)
    Q_UNUSED(master)

    Q_EMIT forked(0);
    return qApp->exec();
}

void WindowsFork::killChild()
{
    if (m_masterChildProcess) {
        m_masterChildProcess->kill();
    }
}

void WindowsFork::terminateChild()
{
    if (m_masterChildProcess) {
        m_masterChildProcess->terminate();
    }
}

void WindowsFork::restart()
{
    restartTerminate();

    if (!m_materChildRestartTimer) {
        m_materChildRestartTimer = new QTimer(this);
        m_materChildRestartTimer->setInterval(1s);
        m_materChildRestartTimer->setSingleShot(false);

        connect(m_materChildRestartTimer, &QTimer::timeout, this, &WindowsFork::restartTerminate);
    }
    m_materChildRestartTimer->start();
}

void WindowsFork::startChild()
{
    m_masterChildProcess->start();
}

void WindowsFork::childFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qCDebug(C_SERVER_WIN) << "Master child finished" << exitCode << exitStatus;
    if (m_materChildRestartTimer || exitStatus == QProcess::CrashExit) {
        startChild();
    } else {
        qApp->exit(exitCode);
    }
}

void WindowsFork::restartTerminate()
{
    if (++m_autoReloadCount > 5) {
        killChild();
    } else {
        terminateChild();
    }
}
