/*
 * SPDX-FileCopyrightText: (C) 2016-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_global.h>

#include <QObject>

class QCoreApplication;

namespace Cutelyst {

class Application;
class ServerPrivate;
/**
 * \ingroup server
 * \class Server server.h Cutelyst/Server/server.h
 * \brief Implements a web server.
 *
 * The %Server class implements a web server that can either act on it’s own or
 * behind another server like nginx or Apache. This class is used by \c \serverexec but
 * can also be integrated into your own application to start() and stop() a %Cutelyst
 * server.
 *
 * <h3 id="configfile">Configuration file options</h3>
 * All command line options from \c \serverexec have their counterparts as properties of this class.
 * Using the \c server section of your \ref configuration "application configuration file" you
 * can set this properties via configuration file options. Simply use the property names as
 * configuration keys.
 * <h4>Example</h4>
 * \code{.ini}
 * [server]
 * http_socket="localhost:3001"
 * threads=4
 * \endcode
 *
 * <h3 id="logging">Logging</h3>
 * The %Cutelyst server uses the following logging categories:
 * \arg cutelyst.server
 * \arg cutelyst.server.engine
 * \arg cutelyst.server.fork
 * \arg cutelyst.server.proto
 * \arg cutelyst.server.fcgi
 * \arg cutelyst.server.http
 * \arg cutelyst.server.http2
 * \arg cutelyst.server.websocket
 * \arg cutelyst.server.socket
 * \arg cutelyst.server.staticmap
 * \arg cutelyst.server.systemd
 * \arg cutelyst.server.tcp
 * \arg cutelyst.server.tcpbalancer
 * \arg cutelyst.server.unix
 * \arg cutelyst.server.windows
 *
 * \sa \ref logging
 */
class CUTELYST_SERVER_EXPORT Server : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Server)
public:
    /**
     * Constructs a new %Server object with the given \a parent.
     *
     * \note When on Linux the constructor will try install our EPoll
     * event loop, so creating this class must be done before creating
     * a QCoreApplition or any of it’s subclasses.
     */
    explicit Server(QObject *parent = nullptr);

    /**
     * Destroys the %Server object.
     */
    virtual ~Server();

    /**
     * Parses \a args from the command line and sets the %Server properties.
     * This will take a list returned by eg. QCoreApplication::arguments().
     */
    void parseCommandLine(const QStringList &args);

    /**
     * This function will start the %server.
     *
     * If an application \a app is provided, it will ignore the value of
     * \link Server::application setApplication\endlink and/or the
     * <A HREF="#configfile">application configuration key</A> in case
     * \link Server::ini ini\endlink or \link Server::json json\endlink
     * is set, meaning it won’t dynamically load an Application
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
     * @note This method does not take ownership of application \a app.
     */
    int exec(Cutelyst::Application *app = nullptr);

    /**
     * This function will start the server in user application mode.
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
     * @note This method does not take ownership of application \a app.
     */
    bool start(Cutelyst::Application *app = nullptr);

    /**
     * Terminates the server execution, when started with start(),
     * it does nothing when started by exec().
     */
    void stop();

    /**
     * Defines application file path to be loaded, an alternative is to provide
     * the Cutelyst::Application pointer to exec().
     * @accessors application(), setApplication()
     */
    Q_PROPERTY(QString application READ application WRITE setApplication NOTIFY changed)
    void setApplication(const QString &application);
    [[nodiscard]] QString application() const;

    /**
     * Defines the number of threads to use, if set to "auto" the ideal thread count is used.
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
    [[nodiscard]] QString threads() const;

    /**
     * Defines the number of processes to use, if set to "auto" the ideal processes count is used.
     * @accessors threads(), setThreads()
     * \note UNIX only
     */
    Q_PROPERTY(QString processes READ processes WRITE setProcesses NOTIFY changed)
    void setProcesses(const QString &process);
    [[nodiscard]] QString processes() const;

    /**
     * Defines directory to change into before application loading.
     * @accessors chdir(), setChdir()
     */
    Q_PROPERTY(QString chdir READ chdir WRITE setChdir NOTIFY changed)
    void setChdir(const QString &chdir);
    [[nodiscard]] QString chdir() const;

    /**
     * Defines how an HTTP socket should be binded.
     * @accessors httpSocket(), setHttpSocket()
     */
    Q_PROPERTY(QStringList http_socket READ httpSocket WRITE setHttpSocket NOTIFY changed)
    void setHttpSocket(const QStringList &httpSocket);
    [[nodiscard]] QStringList httpSocket() const;

    /**
     * Defines how an HTTP2 socket should be binded.
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(QStringList http2_socket READ http2Socket WRITE setHttp2Socket NOTIFY changed)
    void setHttp2Socket(const QStringList &http2Socket);
    [[nodiscard]] QStringList http2Socket() const;

    /**
     * Defines the HTTP2 header table size (SETTINGS_HEADER_TABLE_SIZE) default value: 4096.
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(quint32 http2_header_table_size READ http2HeaderTableSize WRITE
                   setHttp2HeaderTableSize NOTIFY changed)
    void setHttp2HeaderTableSize(quint32 headerTableSize);
    [[nodiscard]] quint32 http2HeaderTableSize() const;

    /**
     * Defines if an HTTP/1 connection can be upgraded to H2C (HTTP 2 Clear Text).
     * Defaults to \c false.
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(bool upgrade_h2c READ upgradeH2c WRITE setUpgradeH2c NOTIFY changed)
    void setUpgradeH2c(bool enable);
    [[nodiscard]] bool upgradeH2c() const;

    /**
     * Defines if HTTPS socket should use ALPN to negotiate HTTP/2.
     * Defaults to \c false.
     * @accessors http2Socket(), setHttp2Socket()
     */
    Q_PROPERTY(bool https_h2 READ httpsH2 WRITE setHttpsH2 NOTIFY changed)
    void setHttpsH2(bool enable);
    [[nodiscard]] bool httpsH2() const;

    /**
     * Defines how an HTTPS socket should be binded.
     * @accessors httpsSocket(), setHttpsSocket()
     */
    Q_PROPERTY(QStringList https_socket READ httpsSocket WRITE setHttpsSocket NOTIFY changed)
    void setHttpsSocket(const QStringList &httpsSocket);
    [[nodiscard]] QStringList httpsSocket() const;

    /**
     * Defines how an FastCGI socket should be binded.
     * @accessors fastcgiSocket(), setFastcgiSocket()
     */
    Q_PROPERTY(QStringList fastcgi_socket READ fastcgiSocket WRITE setFastcgiSocket NOTIFY changed)
    void setFastcgiSocket(const QStringList &fastcgiSocket);
    [[nodiscard]] QStringList fastcgiSocket() const;

    /**
     * Defines the file permissions of a local socket, u = user, g = group, o = others.
     * @accessors socketAccess(), setSocketAccess()
     */
    Q_PROPERTY(QString socket_access READ socketAccess WRITE setSocketAccess NOTIFY changed)
    void setSocketAccess(const QString &socketAccess);
    [[nodiscard]] QString socketAccess() const;

    /**
     * Defines internal socket timeout in seconds.
     * Defaults to \c 4.
     * @accessors socketTimeout(), setSocketTimeout()
     */
    Q_PROPERTY(int socket_timeout READ socketTimeout WRITE setSocketTimeout NOTIFY changed)
    void setSocketTimeout(int timeout);
    [[nodiscard]] int socketTimeout() const;

    /**
     * Defines directory to change into after application loading.
     * @accessors chdir2(), setChdir2()
     */
    Q_PROPERTY(QString chdir2 READ chdir2 WRITE setChdir2 NOTIFY changed)
    void setChdir2(const QString &chdir2);
    [[nodiscard]] QString chdir2() const;

    /**
     * Load config from INI files that will be read by QSettings. When loading multiple files,
     * content will be merged and same keys in the sections will be overwritten by content from
     * later files.
     *
     * @code{.ini}
     * [Cutelst]
     * home="/path/to/my/home"
     *
     * [OtherSection]
     * key=value
     * @endcode
     *
     * @accessors ini(), setIni()
     * @sa config()
     * @sa @ref configuration
     */
    Q_PROPERTY(QStringList ini READ ini WRITE setIni NOTIFY changed)
    void setIni(const QStringList &files);
    [[nodiscard]] QStringList ini() const;

    /**
     * Load config from JSON files containing a JSON object. When loading multiple files, content
     * will be merged and same keys int the sections will be overwritten by content from later
     * files.
     *
     * This is only tested for one single root object with flat child objects as config sections.
     * @code{.json}
     * {
     *      "Cutelyst": {
     *          "home": "/path/to/my/home",
     *          ...
     *      },
     *      "OtherSection": {
     *          "key": "value",
     *          ...
     *      }
     * }
     * @endcode
     *
     * @accessors json(), setJson()
     * @sa config()
     * @sa @ref configuration
     */
    Q_PROPERTY(QStringList json READ json WRITE setJson NOTIFY changed)
    void setJson(const QStringList &files);
    [[nodiscard]] QStringList json() const;

    /**
     * Defines a list of mountpoint to local path mappings to serve static files. Entries have
     * to be in the form <tt>“/mountpoint=/path/to/local/dir”</tt>. If there is then a request
     * for eg. <tt>/mountpoint/css/style.css</tt>, the %Server will remove the mountpoint from the
     * request path and will append the rest to the local path to try to find the requested file,
     * like <tt>/path/to/local/dir/css/style.css</tt>.
     *
     * Added mappings are automatically sorted by the string length of the mounpoint part from
     * short to long and will be compared to the request path in that order.
     *
     * @accessors staticMap(), setStaticMap()
     * @sa @ref servestatic
     */
    Q_PROPERTY(QStringList static_map READ staticMap WRITE setStaticMap NOTIFY changed)
    void setStaticMap(const QStringList &staticMap);
    [[nodiscard]] QStringList staticMap() const;

    /**
     * Defines a list of mountpoint to local path mappings to serve static files. Entries have
     * to be in the form <tt>“/mountpoint=/path/to/local/dir”</tt>. If there ist then a request
     * for eg. <tt>/mountpoint/js/script.js</tt>, the %Server will completely append the request
     * path to the local path to try to find the requested file, like
     * <tt>/path/to/local/dir/mountpoint/js/script.js</tt>.
     *
     * Added mappings are automatically sorted by the string length of the mounpoint part from
     * short to long and will be compared to the request path in that order.
     *
     * @accessors staticMap2(), setStaticMap2()
     * @sa @ref servestatic
     */
    Q_PROPERTY(QStringList static_map2 READ staticMap2 WRITE setStaticMap2 NOTIFY changed)
    void setStaticMap2(const QStringList &staticMap);
    [[nodiscard]] QStringList staticMap2() const;

    /**
     * Defines if a master process should be created to watch for it’s
     * child processes.
     * @accessors master(), setMaster()
     */
    Q_PROPERTY(bool master READ master WRITE setMaster NOTIFY changed)
    void setMaster(bool enable);
    [[nodiscard]] bool master() const;

    /**
     * Reload application if the application file is modified or touched.
     * @accessors autoReload(), setAutoReload()
     */
    Q_PROPERTY(bool auto_reload READ autoReload WRITE setAutoReload NOTIFY changed)
    void setAutoReload(bool enable);
    [[nodiscard]] bool autoReload() const;

    /**
     * Reload application if one of the specified files is modified or touched.
     * @accessors touchReload(), setTouchReload()
     */
    Q_PROPERTY(QStringList touch_reload READ touchReload WRITE setTouchReload NOTIFY changed)
    void setTouchReload(const QStringList &files);
    [[nodiscard]] QStringList touchReload() const;

    /**
     * Defines the socket listen queue size.
     * This setting currently works only on Linux for TCP sockets.
     * Default value: \c 100.
     *
     * @accessors listenQueue(), setListenQueue()
     */
    Q_PROPERTY(int listen READ listenQueue WRITE setListenQueue NOTIFY changed)
    void setListenQueue(int size);
    [[nodiscard]] int listenQueue() const;

    /**
     * Defines the buffer size in bytes used when parsing requests.
     * Default value: \c 4096.
     * @accessors bufferSize(), setBufferSize()
     */
    Q_PROPERTY(int buffer_size READ bufferSize WRITE setBufferSize NOTIFY changed)
    void setBufferSize(int size);
    [[nodiscard]] int bufferSize() const;

    /**
     * Defines the maximum buffer size in bytes of POST request. If a request has a content length
     * that is bigger than the post buffer size, a temporary file is created instead.
     * Default value: \c -1.
     * @accessors postBuffering(), setPostBuffering()
     */
    Q_PROPERTY(qint64 post_buffering READ postBuffering WRITE setPostBuffering NOTIFY changed)
    void setPostBuffering(qint64 size);
    [[nodiscard]] qint64 postBuffering() const;

    /**
     * Defines the buffer size in bytes when reading a POST request.
     * Default value: \c 4096.
     * @accessors postBufferingBufsize(), setPostBufferingBufsize()
     */
    Q_PROPERTY(qint64 post_buffering_bufsize READ postBufferingBufsize WRITE setPostBufferingBufsize
                   NOTIFY changed)
    void setPostBufferingBufsize(qint64 size);
    [[nodiscard]] qint64 postBufferingBufsize() const;

    /**
     * Enable TCP NODELAY on each request.
     * @accessors tcpNodelay(), setTcpNodelay()
     */
    Q_PROPERTY(bool tcp_nodelay READ tcpNodelay WRITE setTcpNodelay NOTIFY changed)
    void setTcpNodelay(bool enable);
    [[nodiscard]] bool tcpNodelay() const;

    /**
     * Enable SO_KEEPALIVE for the sockets.
     * @accessors %soKeepalive(), setSoKeepalive()
     */
    Q_PROPERTY(bool so_keepalive READ soKeepalive WRITE setSoKeepalive NOTIFY changed)
    void setSoKeepalive(bool enable);
    [[nodiscard]] bool soKeepalive() const;

    /**
     * Sets the socket send buffer size in bytes at the OS level. This maps to the SO_SNDBUF socket
     * option.
     * Default value: \c -1.
     * @accessors %socketSndbuf(), setSocketSndbuf()
     */
    Q_PROPERTY(int socket_sndbuf READ socketSndbuf WRITE setSocketSndbuf NOTIFY changed)
    void setSocketSndbuf(int value);
    [[nodiscard]] int socketSndbuf() const;

    /**
     * Sets the socket receive buffer size in bytes at the OS level. This maps to the SO_RCVBUF
     * socket option.
     * Default value: \c -1.
     * @accessors %socketRcvbuf(), setSocketRcvbuf()
     */
    Q_PROPERTY(int socket_rcvbuf READ socketRcvbuf WRITE setSocketRcvbuf NOTIFY changed)
    void setSocketRcvbuf(int value);
    [[nodiscard]] int socketRcvbuf() const;

    /**
     * Sets the maximum allowed size of websocket messages (in KiB, default 1024).
     * @accessors %websocketMaxSize(), setWebsocketMaxSize()
     */
    Q_PROPERTY(
        int websocket_max_size READ websocketMaxSize WRITE setWebsocketMaxSize NOTIFY changed)
    void setWebsocketMaxSize(int value);
    [[nodiscard]] int websocketMaxSize() const;

    /**
     * Defines the pid file to be written before privileges drop.
     * @accessors pidfile(), setPidfile()
     */
    Q_PROPERTY(QString pidfile READ pidfile WRITE setPidfile NOTIFY changed)
    void setPidfile(const QString &file);
    [[nodiscard]] QString pidfile() const;

    /**
     * Defines the pid file to be written after privileges drop.
     * @accessors pidfile(), setPidfile()
     */
    Q_PROPERTY(QString pidfile2 READ pidfile2 WRITE setPidfile2 NOTIFY changed)
    void setPidfile2(const QString &file);
    [[nodiscard]] QString pidfile2() const;

    /**
     * Defines user id of the process.
     * @accessors uid(), setUid()
     * \note UNIX only
     */
    Q_PROPERTY(QString uid READ uid WRITE setUid NOTIFY changed)
    void setUid(const QString &uid);
    [[nodiscard]] QString uid() const;

    /**
     * Defines group id of the process.
     * @accessors gid(), setGid()
     * \note UNIX only
     */
    Q_PROPERTY(QString gid READ gid WRITE setGid NOTIFY changed)
    void setGid(const QString &gid);
    [[nodiscard]] QString gid() const;

    /**
     * Disable additional groups set via initgroups().
     * @accessors noInitgroups(), setNoInitgroups()
     * \note UNIX only
     */
    Q_PROPERTY(bool no_initgroups READ noInitgroups WRITE setNoInitgroups NOTIFY changed)
    void setNoInitgroups(bool enable);
    [[nodiscard]] bool noInitgroups() const;

    /**
     * Defines owner of UNIX sockets.
     * @accessors chownSocket(), setChownSocket()
     * \note UNIX only
     */
    Q_PROPERTY(QString chown_socket READ chownSocket WRITE setChownSocket NOTIFY changed)
    void setChownSocket(const QString &chownSocket);
    [[nodiscard]] QString chownSocket() const;

    /**
     * Defines file mode creation mask.
     * @accessors umask(), setUmask()
     * \note UNIX only
     */
    Q_PROPERTY(QString umask READ umask WRITE setUmask NOTIFY changed)
    void setUmask(const QString &value);
    [[nodiscard]] QString umask() const;

    /**
     * Defines CPU affinity.
     * @accessors cpuAffinity(), setCpuAffinity()
     * \note UNIX only
     */
    Q_PROPERTY(int cpu_affinity READ cpuAffinity WRITE setCpuAffinity NOTIFY changed)
    void setCpuAffinity(int value);
    [[nodiscard]] int cpuAffinity() const;

    /**
     * Enable SO_REUSEPORT for the sockets.
     * @accessors reusePort(), setReusePort()
     * \note Linux only
     */
    Q_PROPERTY(bool reuse_port READ reusePort WRITE setReusePort NOTIFY changed)
    void setReusePort(bool enable);
    [[nodiscard]] bool reusePort() const;

    /**
     * Defines is the Application should be lazy loaded.
     * @accessors lazy(), setLazy()
     */
    Q_PROPERTY(bool lazy READ lazy WRITE setLazy NOTIFY changed)
    void setLazy(bool enable);
    [[nodiscard]] bool lazy() const;

    /**
     * Defines if a reverse proxy operates in front of this application server.
     * If enabled, parses the HTTP headers X-Forwarded-For, X-Forwarded-Host and X-Forwarded-Proto
     * and uses this info to update Cutelyst::EngineRequest.
     * @accessors usingFrontendProxy(), setUsingFrontendProxy()
     */
    Q_PROPERTY(bool using_frontend_proxy READ usingFrontendProxy WRITE setUsingFrontendProxy NOTIFY
                   changed)
    void setUsingFrontendProxy(bool enable);
    [[nodiscard]] bool usingFrontendProxy() const;

    /**
     * Returns the configuration set by setIni() and setJson().
     * @since %Cutelyst 4.0.0
     */
    [[nodiscard]] QVariantMap config() const noexcept;

Q_SIGNALS:
    /**
     * It is emitted once the server is ready.
     */
    void ready();

    /**
     * It is emitted once the server shutdown is completed.
     */
    void stopped();

    /**
     * It is emitted once config changes.
     */
    void changed();

    /**
     * It is emitted once error occurs.
     */
    void errorOccured(const QString &error);

protected:
    ServerPrivate *d_ptr;
};

} // namespace Cutelyst
