/*
 * SPDX-FileCopyrightText: (C) 2016-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include "server.h"
#include "serverengine.h"

#include <Cutelyst/Application>

#include <QQueue>

class QTcpServer;
class QSettings;
class AbstractFork;

namespace Cutelyst {

class Protocol;
class ProtocolHttp2;
class ServerPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Server)
public:
    explicit ServerPrivate(Server *parent)
        : QObject(parent)
        , q_ptr(parent)
    {
    }
    ~ServerPrivate();

    enum class ConfigFormat : quint8 { Ini, Json };

    bool listenTcpSockets();
    bool listenTcp(const QString &line, Protocol *protocol, bool secure);
    bool listenLocalSockets();
    bool listenLocal(const QString &line, Protocol *protocol);
    bool setupApplication();
    void engineShutdown(ServerEngine *engine);
    void checkEngineShutdown();
    void workerStarted();
    bool postFork(int workerId);
    bool writePidFile(const QString &filename);

    ServerEngine *createEngine(Cutelyst::Application *app, int core);

    void loadConfig();
    void applyConfig(const QVariantMap &config);
    void loadLoggingRules(QSettings &settings);

    Protocol *getHttpProto();
    ProtocolHttp2 *getHttp2Proto();
    Protocol *getFastCgiProto();

    Server *q_ptr;
    std::vector<QObject *> servers;
    std::vector<ServerEngine *> engines;
    Cutelyst::Application *app = nullptr;
    ServerEngine *mainEngine   = nullptr;

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
    QStringList configLoaded;
    QQueue<std::pair<QString, ConfigFormat>> configToLoad;
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
    bool noInitgroups           = false;
    int cpuAffinity             = 0;
    bool reusePort              = false;
    qint64 postBuffering        = -1;
    qint64 postBufferingBufsize = 4096;
    Protocol *protoHTTP         = nullptr;
    ProtocolHttp2 *protoHTTP2   = nullptr;
    Protocol *protoFCGI         = nullptr;
    AbstractFork *genericFork   = nullptr;
    int bufferSize              = 4096;
    int workersNotRunning       = 1;
    int threads                 = 1;
    int processes               = 0;
    int socketSendBuf           = -1;
    int socketReceiveBuf        = -1;
    int socketTimeout           = 4;
    int websocketMaxSize        = 1024 * 1024;
    int listenQueue             = 100;
    bool lazy                   = false;
    bool master                 = false;
    bool autoReload             = false;
    bool tcpNodelay             = false;
    bool soKeepalive            = false;
    bool threadBalancer         = false;
    bool userEventLoop          = false;
    bool upgradeH2c             = false;
    bool httpsH2                = false;
    bool usingFrontendProxy     = false;
    bool loadingConfig          = false;

Q_SIGNALS:
    void postForked(int workerId);
    void killChildProcess();
    void terminateChildProcess();
    void shutdown();
};

} // namespace Cutelyst
