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
#ifndef WSGI_H
#define WSGI_H

#include <QObject>

#include <Cutelyst/cutelyst_global.h>

class QCoreApplication;

namespace Cutelyst {
class Application;
}

namespace CWSGI {

class WSGIPrivate;
/*! \class WSGI wsgi.h Cutelyst/WSGI
 * \brief Implements a %WSGI server.
 *
 */
class CUTELYST_WSGI_EXPORT WSGI : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(WSGI)
public:
    explicit WSGI(QObject *parent = nullptr);
    virtual ~WSGI();

    /**
     * This function will start the WSGI server.
     *
     * If an application is provided it will ignore the value of
     * setApplication and/or the Application configuration in case
     * Ini is set, meaning it won't dynamically load an Application
     * but use this to create new instances (if the app constructor is
     * marked as Q_INVOKABLE).
     *
     * It will return 0 in case of sucess.
     *
     * @note This method calls QCoreApplication::exec() internally,
     * this is needed because when creating or recreating child process
     * the event loop must not be running otherwise we get undefined
     * behavior. So exit main after this function.
     */
    int exec(Cutelyst::Application *app = nullptr);

    Q_PROPERTY(QString application READ application WRITE setApplication)
    void setApplication(const QString &application);
    QString application() const;

    /**
     * Defines the number of threads to use, if set to "auto" the ideal thread count is used
     * @accessors threads(), setThreads()
     */
    Q_PROPERTY(QString threads READ threads WRITE setThreads)
    void setThreads(const QString &threads);
    QString threads() const;

#ifdef Q_OS_UNIX
    /**
     * Defines the number of processes to use, if set to "auto" the ideal processes count is used
     * @accessors threads(), setThreads()
     */
    Q_PROPERTY(QString processes READ processes WRITE setProcesses)
    void setProcesses(const QString &process);
    QString processes() const;
#endif

    /**
     * Defines directory to chdir to before application loading
     * @accessors chdir(), setChdir()
     */
    Q_PROPERTY(QString chdir READ chdir WRITE setChdir)
    void setChdir(const QString &chdir);
    QString chdir() const;

    Q_PROPERTY(QString http_socket READ httpSocket WRITE setHttpSocket)
    void setHttpSocket(const QString &httpSocket);
    QString httpSocket() const;

    Q_PROPERTY(QString fastcgi_socket READ fastcgiSocket WRITE setFastcgiSocket)
    void setFastcgiSocket(const QString &fastcgiSocket);
    QString fastcgiSocket() const;

    Q_PROPERTY(QString socket_access READ socketAccess WRITE setSocketAccess)
    void setSocketAccess(const QString &socketAccess);
    QString socketAccess() const;

    /**
     * Defines set internal socket timeout
     * @accessors socketTimeout(), setSocketTimeout()
     */
    Q_PROPERTY(int socket_timeout READ socketTimeout WRITE setSocketTimeout)
    void setSocketTimeout(int timeout);
    int socketTimeout() const;

    /**
     * Defines directory to chdir to after application loading
     * @accessors chdir2(), setChdir2()
     */
    Q_PROPERTY(QString chdir2 READ chdir2 WRITE setChdir2)
    void setChdir2(const QString &chdir2);
    QString chdir2() const;

    /**
     * Load config from ini file
     * @accessors ini(), setIni()
     */
    Q_PROPERTY(QString ini READ ini WRITE setIni)
    void setIni(const QString &ini);
    QString ini() const;

    /**
     * Map the mountpoint to static directory (or file)
     * @accessors staticMap(), setStaticMap()
     */
    Q_PROPERTY(QString static_map READ staticMap WRITE setStaticMap)
    void setStaticMap(const QString &staticMap);
    QString staticMap() const;

    /**
     * Map the mountpoint to static directory (or file), completely appending the requested resource to the docroot
     * @accessors staticMap2(), setStaticMap2()
     */
    Q_PROPERTY(QString static_map2 READ staticMap2 WRITE setStaticMap2)
    void setStaticMap2(const QString &staticMap);
    QString staticMap2() const;

    Q_PROPERTY(bool master READ master WRITE setMaster)
    void setMaster(bool enable);
    bool master() const;

    /**
     * Reload application if the application file is modified or touched
     * @accessors autoReload(), setAutoReload()
     */
    Q_PROPERTY(bool auto_reload READ autoReload WRITE setAutoReload)
    void setAutoReload(bool enable);
    bool autoReload() const;

    /**
     * Reload application if the specified file is modified or touched
     * @accessors touchReload(), setTouchReload()
     */
    Q_PROPERTY(QString touch_reload READ touchReload WRITE setTouchReload)
    void setTouchReload(const QString &file);
    QString touchReload() const;

    Q_PROPERTY(qint64 buffer_size READ bufferSize WRITE setBufferSize)
    void setBufferSize(qint64 size);
    int bufferSize() const;

    Q_PROPERTY(qint64 post_buffering READ postBuffering WRITE setPostBuffering)
    void setPostBuffering(qint64 size);
    qint64 postBuffering() const;

    Q_PROPERTY(qint64 post_buffering_bufsize READ postBufferingBufsize WRITE setPostBufferingBufsize)
    void setPostBufferingBufsize(qint64 size);
    qint64 postBufferingBufsize() const;

    /**
     * Enable TCP NODELAY on each request
     * @accessors tcpNodelay(), setTcpNodelay()
     */
    Q_PROPERTY(bool tcp_nodelay READ tcpNodelay WRITE setTcpNodelay)
    void setTcpNodelay(bool enable);
    bool tcpNodelay() const;

    Q_PROPERTY(bool so_keepalive READ soKeepalive WRITE setSoKeepalive)
    void setSoKeepalive(bool enable);
    bool soKeepalive() const;

    /**
     * Sets the socket send buffer size in bytes at the OS level. This maps to the SO_SNDBUF socket option
     * @accessors %socketSndbuf(), setSocketSndbuf()
     */
    Q_PROPERTY(int socket_sndbuf READ socketSndbuf WRITE setSocketSndbuf)
    void setSocketSndbuf(int value);
    int socketSndbuf() const;

    /**
     * Sets the socket receive buffer size in bytes at the OS level. This maps to the SO_RCVBUF socket option
     * @accessors %socketRcvbuf(), setSocketRcvbuf()
     */
    Q_PROPERTY(int socket_rcvbuf READ socketRcvbuf WRITE setSocketRcvbuf)
    void setSocketRcvbuf(int value);
    int socketRcvbuf() const;

#ifdef Q_OS_UNIX
    /**
     * Defines user id of the process.
     * @accessors uid(), setUid()
     */
    Q_PROPERTY(QString uid READ uid WRITE setUid)
    void setUid(const QString &uid);
    QString uid() const;

    /**
     * Defines group id of the process.
     * @accessors gid(), setGid()
     */
    Q_PROPERTY(QString gid READ gid WRITE setGid)
    void setGid(const QString &gid);
    QString gid() const;

    /**
     * Defines owner of UNIX sockets.
     * @accessors chownSocket(), setChownSocket()
     */
    Q_PROPERTY(QString chown_socket READ chownSocket WRITE setChownSocket)
    void setChownSocket(const QString &chownSocket);
    QString chownSocket() const;

    /**
     * Defines is the Application should be lazy loaded.
     * @accessors lazy(), setLazy()
     */
    Q_PROPERTY(bool lazy READ lazy WRITE setLazy)
    void setLazy(bool enable);
    bool lazy() const;
#endif

Q_SIGNALS:
    /**
     * It is emitted once the server is ready.
     */
    void ready();

protected:
    WSGIPrivate *d_ptr;
};

}

#endif // WSGI_H
