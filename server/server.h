/*
 * SPDX-FileCopyrightText: (C) 2016-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTSERVER_H
#define CUTELYSTSERVER_H

#include <Cutelyst/cutelyst_global.h>

#include <QObject>

class QCoreApplication;

namespace Cutelyst {

class Application;
class ServerPrivate;
/*! \class WSGI wsgi.h Cutelyst/Server
 * \brief Implements a %WSGI server.
 *
 */
class CUTELYST_SERVER_EXPORT Server : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Server)
public:
    /**
     * @brief Server
     *
     * \note When on Linux the constructor will try install our EPoll
     * event loop, so creating this class must be done before creating
     * a QCoreApplition or any of it's subclasses.
     * @param parent
     */
    explicit Server(QObject *parent = nullptr);
    virtual ~Server();

    void parseCommandLine(const QStringList &args);

    /**
     * This function will start the WSGI server.
     *
     * If an application is provided it will ignore the value of
     * setApplication and/or the Application configuration in case
     * Ini is set, meaning it won't dynamically load an Application
     * but use this to create new instances (if the app constructor is
     * marked as Q_INVOKABLE and threads settings are greater than 1).
     *
     * It will return 0 in case of sucess.
     *
     * @note This method calls QCoreApplication::exec() internally,
     * this is needed because when creating or recreating child process
     * the event loop must not be running otherwise we get undefined
     * behavior. So exit main after this function.
     *
     * @note This method does not take ownership of application \pa appp
     */
    int exec(Cutelyst::Application *app = nullptr);

    /*!
     * This function will start the Cutelyst::Server in user application mode.
     *
     * Use this when you would like to embed the server in an
     * application that is able to start/stop the server at will, for
     * example with a push button.
     *
     * This method does not support forking which includes master, lazy or processes
     * properties.
     *
     * New application instances will be created if the app constructor is
     * marked as Q_INVOKABLE and threads settings are greater than 1.
     *
     * @note This method does not take ownership of application \pa appp
     */
    bool start(Cutelyst::Application *app = nullptr);

    /*!
     * Terminates the server execution, when started with start(),
     * it does nothing when started by exec().
     */
    void stop();

    /**
     * Defines application file path to be loaded, an alternative is to provide
     * the Cutelyst::Application pointer to exec()
     * @accessors application(), setApplication()
     */
    Q_PROPERTY(QString application READ application WRITE setApplication NOTIFY changed)
    void setApplication(const QString &application);
    QString application() const;

    /**
     * Defines the number of threads to use, if set to "auto" the ideal thread count is used
     * @accessors threads(), setThreads()
     *
     * A new thread is only created when > "2" or if "auto" reports more than 1 core, when
     * the number of threads is 2 or more a new thread is not created for the worker 0,
     * it's this way to save allocating a new Application as we already have our current
     * thread.
     *
     * If it's desired to not have the Server running on the GUI (or current) thread for
     * example, the Server must be moved to a new thread manually.
     */
    Q_PROPERTY(QString threads READ threads WRITE setThreads NOTIFY changed)
    void setThreads(const QString &threads);
    QString threads() const;

    /**
     * Defines the number of processes to use, if set to "auto" the ideal processes count is used
     * @accessors threads(), setThreads()
     * \note UNIX only
     */
    Q_PROPERTY(QString processes READ processes WRITE setProcesses NOTIFY changed)
    void setProcesses(const QString &process);
    QString processes() const;

    /**
     * Defines directory to chdir to before application loading
     * @accessors chdir(), setChdir()
     */
    Q_PROPERTY(QString chdir READ chdir WRITE setChdir NOTIFY changed)
    void setChdir(const QString &chdir);
    QString chdir() const;

    /**
     * Defines how an HTTP socket should be binded
     * @accessors httpSocket(), setHttpSocket()
     */
    Q_PROPERTY(QStringList http_socket READ httpSocket WRITE setHttpSocket NOTIFY changed)
    void setHttpSocket(const QStringList &httpSocket);
    QStringList httpSocket() const;

    /**
     * Defines how an HTTP2 socket should be binded
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(QStringList http2_socket READ http2Socket WRITE setHttp2Socket NOTIFY changed)
    void setHttp2Socket(const QStringList &http2Socket);
    QStringList http2Socket() const;

    /**
     * Defines the HTTP2 header table size (SETTINGS_HEADER_TABLE_SIZE) default value: 4096
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(quint32 http2_header_table_size READ http2HeaderTableSize WRITE setHttp2HeaderTableSize NOTIFY changed)
    void setHttp2HeaderTableSize(quint32 headerTableSize);
    quint32 http2HeaderTableSize() const;

    /**
     * Defines if an HTTP/1 connection can be upgraded to H2C (HTTP 2 Clear Text)
     * Defaults to false
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(bool upgrade_h2c READ upgradeH2c WRITE setUpgradeH2c NOTIFY changed)
    void setUpgradeH2c(bool enable);
    bool upgradeH2c() const;

    /**
     * Defines if HTTPS sockect should use ALPN to negotiate HTTP/2
     * Defaults to false
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(bool https_h2 READ httpsH2 WRITE setHttpsH2 NOTIFY changed)
    void setHttpsH2(bool enable);
    bool httpsH2() const;

    /**
     * Defines how an HTTPS socket should be binded
     * @accessors httpsSocket(), setHttpsSocket()
     */
    Q_PROPERTY(QStringList https_socket READ httpsSocket WRITE setHttpsSocket NOTIFY changed)
    void setHttpsSocket(const QStringList &httpsSocket);
    QStringList httpsSocket() const;

    /**
     * Defines how an FastCGI socket should be binded
     * @accessors fastcgiSocket(), setFastcgiSocket()
     */
    Q_PROPERTY(QStringList fastcgi_socket READ fastcgiSocket WRITE setFastcgiSocket NOTIFY changed)
    void setFastcgiSocket(const QStringList &fastcgiSocket);
    QStringList fastcgiSocket() const;

    /**
     * Defines the file permissions of a local socket, u = user, g = group, o = others
     * @accessors socketAccess(), setSocketAccess()
     */
    Q_PROPERTY(QString socket_access READ socketAccess WRITE setSocketAccess NOTIFY changed)
    void setSocketAccess(const QString &socketAccess);
    QString socketAccess() const;

    /**
     * Defines set internal socket timeout
     * @accessors socketTimeout(), setSocketTimeout()
     */
    Q_PROPERTY(int socket_timeout READ socketTimeout WRITE setSocketTimeout NOTIFY changed)
    void setSocketTimeout(int timeout);
    int socketTimeout() const;

    /**
     * Defines directory to chdir to after application loading
     * @accessors chdir2(), setChdir2()
     */
    Q_PROPERTY(QString chdir2 READ chdir2 WRITE setChdir2 NOTIFY changed)
    void setChdir2(const QString &chdir2);
    QString chdir2() const;

    /**
     * Load config from ini file
     * @accessors ini(), setIni()
     */
    Q_PROPERTY(QStringList ini READ ini WRITE setIni NOTIFY changed)
    void setIni(const QStringList &files);
    QStringList ini() const;

    /**
     * Load config from JSON file
     * @accessors json(), setJson()
     */
    Q_PROPERTY(QStringList json READ json WRITE setJson NOTIFY changed)
    void setJson(const QStringList &files);
    QStringList json() const;

    /**
     * Map the mountpoint to static directory (or file)
     * @accessors staticMap(), setStaticMap()
     */
    Q_PROPERTY(QStringList static_map READ staticMap WRITE setStaticMap NOTIFY changed)
    void setStaticMap(const QStringList &staticMap);
    QStringList staticMap() const;

    /**
     * Map the mountpoint to static directory (or file), completely appending the requested resource to the docroot
     * @accessors staticMap2(), setStaticMap2()
     */
    Q_PROPERTY(QStringList static_map2 READ staticMap2 WRITE setStaticMap2 NOTIFY changed)
    void setStaticMap2(const QStringList &staticMap);
    QStringList staticMap2() const;

    /**
     * Defines if a master process should be created to watch for it's
     * child processes
     * @accessors master(), setMaster()
     */
    Q_PROPERTY(bool master READ master WRITE setMaster NOTIFY changed)
    void setMaster(bool enable);
    bool master() const;

    /**
     * Reload application if the application file is modified or touched
     * @accessors autoReload(), setAutoReload()
     */
    Q_PROPERTY(bool auto_reload READ autoReload WRITE setAutoReload NOTIFY changed)
    void setAutoReload(bool enable);
    bool autoReload() const;

    /**
     * Reload application if the specified file is modified or touched
     * @accessors touchReload(), setTouchReload()
     */
    Q_PROPERTY(QStringList touch_reload READ touchReload WRITE setTouchReload NOTIFY changed)
    void setTouchReload(const QStringList &files);
    QStringList touchReload() const;

    /**
     * Defines the socket listen queue size.
     * This setting currently works only on Linux for TCP sockets.
     *
     * @accessors listenQueue(), setListenQueue()
     */
    Q_PROPERTY(int listen READ listenQueue WRITE setListenQueue NOTIFY changed)
    void setListenQueue(int size);
    int listenQueue() const;

    /**
     * Defines the buffer size used when parsing requests
     * @accessors bufferSize(), setBufferSize()
     */
    Q_PROPERTY(int buffer_size READ bufferSize WRITE setBufferSize NOTIFY changed)
    void setBufferSize(int size);
    int bufferSize() const;

    /**
     * Defines the maximum buffer size of POST request, if a request has a content length
     * that is bigger than the post buffer size a temporary file is created instead
     * @accessors postBuffering(), setPostBuffering()
     */
    Q_PROPERTY(qint64 post_buffering READ postBuffering WRITE setPostBuffering NOTIFY changed)
    void setPostBuffering(qint64 size);
    qint64 postBuffering() const;

    /**
     * Defines the buffer size when reading a POST request
     * @accessors postBufferingBufsize(), setPostBufferingBufsize()
     */
    Q_PROPERTY(qint64 post_buffering_bufsize READ postBufferingBufsize WRITE setPostBufferingBufsize NOTIFY changed)
    void setPostBufferingBufsize(qint64 size);
    qint64 postBufferingBufsize() const;

    /**
     * Enable TCP NODELAY on each request
     * @accessors tcpNodelay(), setTcpNodelay()
     */
    Q_PROPERTY(bool tcp_nodelay READ tcpNodelay WRITE setTcpNodelay NOTIFY changed)
    void setTcpNodelay(bool enable);
    bool tcpNodelay() const;

    /**
     * Enable SO_KEEPALIVE for the sockets
     * @accessors %soKeepalive(), setSoKeepalive()
     */
    Q_PROPERTY(bool so_keepalive READ soKeepalive WRITE setSoKeepalive NOTIFY changed)
    void setSoKeepalive(bool enable);
    bool soKeepalive() const;

    /**
     * Sets the socket send buffer size in bytes at the OS level. This maps to the SO_SNDBUF socket option
     * @accessors %socketSndbuf(), setSocketSndbuf()
     */
    Q_PROPERTY(int socket_sndbuf READ socketSndbuf WRITE setSocketSndbuf NOTIFY changed)
    void setSocketSndbuf(int value);
    int socketSndbuf() const;

    /**
     * Sets the socket receive buffer size in bytes at the OS level. This maps to the SO_RCVBUF socket option
     * @accessors %socketRcvbuf(), setSocketRcvbuf()
     */
    Q_PROPERTY(int socket_rcvbuf READ socketRcvbuf WRITE setSocketRcvbuf NOTIFY changed)
    void setSocketRcvbuf(int value);
    int socketRcvbuf() const;

    /**
     * Sets the maximum allowed size of websocket messages (in Kbytes, default 1024)
     * @accessors %websocketMaxSize(), setWebsocketMaxSize()
     */
    Q_PROPERTY(int websocket_max_size READ websocketMaxSize WRITE setWebsocketMaxSize NOTIFY changed)
    void setWebsocketMaxSize(int value);
    int websocketMaxSize() const;

    /**
     * Defines the pid file to be written before privileges drop
     * @accessors pidfile(), setPidfile()
     */
    Q_PROPERTY(QString pidfile READ pidfile WRITE setPidfile NOTIFY changed)
    void setPidfile(const QString &file);
    QString pidfile() const;

    /**
     * Defines the pid file to be written before privileges drop
     * @accessors pidfile(), setPidfile()
     */
    Q_PROPERTY(QString pidfile2 READ pidfile2 WRITE setPidfile2 NOTIFY changed)
    void setPidfile2(const QString &file);
    QString pidfile2() const;

    /**
     * Defines user id of the process.
     * @accessors uid(), setUid()
     * \note UNIX only
     */
    Q_PROPERTY(QString uid READ uid WRITE setUid NOTIFY changed)
    void setUid(const QString &uid);
    QString uid() const;

    /**
     * Defines group id of the process.
     * @accessors gid(), setGid()
     * \note UNIX only
     */
    Q_PROPERTY(QString gid READ gid WRITE setGid NOTIFY changed)
    void setGid(const QString &gid);
    QString gid() const;

    /**
     * Disable additional groups set via initgroups()
     * @accessors noInitgroups(), setNoInitgroups()
     * \note UNIX only
     */
    Q_PROPERTY(bool no_initgroups READ noInitgroups WRITE setNoInitgroups NOTIFY changed)
    void setNoInitgroups(bool enable);
    bool noInitgroups() const;

    /**
     * Defines owner of UNIX sockets.
     * @accessors chownSocket(), setChownSocket()
     * \note UNIX only
     */
    Q_PROPERTY(QString chown_socket READ chownSocket WRITE setChownSocket NOTIFY changed)
    void setChownSocket(const QString &chownSocket);
    QString chownSocket() const;

    /**
     * Defines file mode creation mask
     * @accessors umask(), setUmask()
     * \note UNIX only
     */
    Q_PROPERTY(QString umask READ umask WRITE setUmask NOTIFY changed)
    void setUmask(const QString &value);
    QString umask() const;

    /**
     * Defines CPU affinity
     * @accessors cpuAffinity(), setCpuAffinity()
     * \note UNIX only
     */
    Q_PROPERTY(int cpu_affinity READ cpuAffinity WRITE setCpuAffinity NOTIFY changed)
    void setCpuAffinity(int value);
    int cpuAffinity() const;

    /**
     * Enable SO_REUSEPORT for the sockets
     * @accessors reusePort(), setReusePort()
     * \note Linux only
     */
    Q_PROPERTY(bool reuse_port READ reusePort WRITE setReusePort NOTIFY changed)
    void setReusePort(bool enable);
    bool reusePort() const;

    /**
     * Defines is the Application should be lazy loaded.
     * @accessors lazy(), setLazy()
     */
    Q_PROPERTY(bool lazy READ lazy WRITE setLazy NOTIFY changed)
    void setLazy(bool enable);
    bool lazy() const;

    /**
     * Defines if a reverse proxy operates in front of this application server.
     * If enabled, parses the http headers X-Forwarded-For, X-Forwarded-Host and X-Forwarded-Proto
     * and uses this info to update Cutelyst::EngineRequest
     * @accessors usingFrontendProxy(), setUsingFrontendProxy()
     */
    Q_PROPERTY(bool using_frontend_proxy READ usingFrontendProxy WRITE setUsingFrontendProxy NOTIFY changed)
    void setUsingFrontendProxy(bool enable);
    bool usingFrontendProxy() const;

Q_SIGNALS:
    /**
     * It is emitted once the server is ready.
     */
    void ready();

    /**
     * It is emitted once the server shutdown is completed.
     */
    void stopped();

    void changed();

    /**
     * It is emitted once error occurs.
     */
    void errorOccured(const QString &error);

protected:
    ServerPrivate *d_ptr;
};

} // namespace Cutelyst

#endif // CUTELYSTSERVER_H
