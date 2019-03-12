/*
 * Copyright (C) 2016-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef WSGI_P_H
#define WSGI_P_H

#include "wsgi.h"

#include "cwsgiengine.h"

#include <Cutelyst/Application>

class QTcpServer;
class QSettings;
class AbstractFork;

namespace CWSGI {

class Protocol;
class ProtocolHttp2;
class WSGIPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(WSGI)
public:
    inline WSGIPrivate(WSGI *parent) : QObject(parent), q_ptr(parent) { }

    void listenTcpSockets();
    bool listenTcp(const QString &line, Protocol *protocol, bool secure);
    void listenLocalSockets();
    bool listenLocal(const QString &line, Protocol *protocol);
    void setupApplication();
    void engineShutdown(CWsgiEngine *engine);
    void workerStarted();
    void postFork(int workerId);
    void writePidFile(const QString &filename);

    CWsgiEngine *createEngine(Cutelyst::Application *app, int core);

    void loadConfig(const QString &file, bool json);
    void applyConfig(const QVariantMap &config);
    void loadLoggingRules(QSettings &settings);

    Protocol *getHttpProto();
    ProtocolHttp2 *getHttp2Proto();
    Protocol *getFastCgiProto();

    WSGI *q_ptr;
    std::vector<QObject *> servers;
    std::vector<CWsgiEngine *> engines;
    Cutelyst::Application *app = nullptr;
    CWsgiEngine *engine = nullptr;

    QVariantMap opt;
    QVariantMap config;
    QStringList httpSockets;
    QStringList http2Sockets;
    quint32 http2HeaderTableSize = 4096;
    QStringList httpsSockets;
    QStringList fastcgiSockets;
    QStringList staticMaps;
    QStringList staticMaps2;
    QStringList touchReload;
    QStringList ini;
    QStringList json;
    QString application;
    QString chdir;
    QString chdir2;
    QString socketAccess;
    QString pidfile;
    QString pidfile2;
    QString uid;
    QString gid;
    QString chownSocket;
    QString umask;
    bool noInitgroups = false;
    int cpuAffinity = 0;
    bool reusePort = false;
    qint64 postBuffering = -1;
    qint64 postBufferingBufsize = 4096;
    Protocol *protoHTTP = nullptr;
    ProtocolHttp2 *protoHTTP2 = nullptr;
    Protocol *protoFCGI = nullptr;
    AbstractFork *genericFork = nullptr;
    int bufferSize = 4096;
    int workersNotRunning = 1;
    int threads = 1;
    int processes = 0;
    int socketSendBuf = -1;
    int socketReceiveBuf = -1;
    int socketTimeout = 4;
    int websocketMaxSize = 1024 * 1024;
    bool lazy = false;
    bool master = false;
    bool autoReload = false;
    bool tcpNodelay = false;
    bool soKeepalive = false;
    bool threadBalancer = false;
    bool userEventLoop = false;
    bool upgradeH2c = false;
    bool httpsH2 = false;
    bool usingFrontendProxy = false;

Q_SIGNALS:
    void postForked(int workerId);
    void killChildProcess();
    void terminateChildProcess();
    void shutdown();
};

}

#endif // WSGI_P_H
