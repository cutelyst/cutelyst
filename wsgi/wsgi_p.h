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
#ifndef WSGI_P_H
#define WSGI_P_H

#include "wsgi.h"

#include "cwsgiengine.h"

#include <QProcess>

#include <Cutelyst/Application>

class QTcpServer;
class QSettings;
class QTimer;

#ifdef Q_OS_UNIX
class UnixFork;
#endif

namespace CWSGI {

class CWsgiEngine;
class WSGIPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(WSGI)
public:
    inline WSGIPrivate(WSGI *parent) : QObject(parent), q_ptr(parent) { }

    void listenTcpSockets();
    bool listenTcp(const QString &line, Protocol *protocol);
    void listenLocalSockets();
    bool listenLocal(const QString &line, Protocol *protocol);
    bool proc();
    void parseCommandLine();
    int setupApplication(Cutelyst::Application *app);
    void childFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void restart(const QString &path);
    void restartTerminate();
    void engineInitted();
    void engineShutdown(CWsgiEngine *engine);
    void workerStarted();

    CWsgiEngine *createEngine(Cutelyst::Application *app, int core);

    bool loadConfig();
    void loadConfigGroup(const QString &group, QSettings &settings);
    void loadLoggingRules(QSettings &settings);

    WSGI *q_ptr;
    std::vector<SocketInfo> sockets;
    std::vector<Cutelyst::Engine *> engines;
    CWsgiEngine *engine;

    QStringList httpSockets;
    QStringList fastcgiSockets;
    QStringList staticMaps;
    QStringList staticMaps2;
    QStringList touchReload;
    QString application;
    QString chdir;
    QString chdir2;
    QString ini;
    QString socketAccess;
#ifdef Q_OS_UNIX
    QString uid;
    QString gid;
    QString chownSocket;
#endif
    qint64 postBuffering = -1;
    qint64 postBufferingBufsize = 4096;
    QProcess *masterChildProcess = nullptr;
    QTimer *materChildRestartTimer = nullptr;
    Protocol *protoHTTP = nullptr;
    Protocol *protoFCGI = nullptr;
#ifdef Q_OS_UNIX
    UnixFork *unixFork = nullptr;
#endif
    int bufferSize = 4096;
    int enginesInitted = 1;
    int workersNotRunning = 1;
    int threads = 0;
    int process = 0;
    int socketSendBuf = -1;
    int socketReceiveBuf = -1;
    int autoReloadCount = 0;
    bool master = false;
    bool autoReload = false;
    bool tcpNodelay = false;
    bool soKeepalive = false;

Q_SIGNALS:
    void forked();
    void killChildProcess();
    void terminateChildProcess();
    void shutdown();
};

}

#endif // WSGI_P_H

