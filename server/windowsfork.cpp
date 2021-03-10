/*
 * Copyright (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "windowsfork.h"

#include <windows.h>

#include <QLoggingCategory>
#include <QCoreApplication>
#include <QTimer>

Q_LOGGING_CATEGORY(WSGI_WIN, "wsgi.win", QtWarningMsg)

WindowsFork::WindowsFork(QObject *parent) : AbstractFork(parent)
{

}

bool WindowsFork::continueMaster(int *exit)
{
    installTouchReload();

    m_masterChildProcess = new QProcess(this);
    connect(m_masterChildProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
            this, &WindowsFork::childFinished);

    auto env = QProcessEnvironment::systemEnvironment();
    env.insert(QStringLiteral("CUTELYST_WSGI_IGNORE_MASTER"), QStringLiteral("1"));
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
        m_materChildRestartTimer->setInterval(1 * 1000);
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
    qCDebug(WSGI_WIN) << "Master child finished" << exitCode << exitStatus;
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
