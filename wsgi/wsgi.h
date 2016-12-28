/*
 * Copyright (C) 2016 Daniel Nicoletti <dantti12@gmail.com>
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
     */
    int load(Cutelyst::Application *app = nullptr);

    Q_PROPERTY(QString application READ application WRITE setApplication)
    void setApplication(const QString &application);
    QString application() const;

    Q_PROPERTY(QString threads READ threads WRITE setThreads)
    void setThreads(const QString &threads);
    QString threads() const;

    Q_PROPERTY(QString process READ process WRITE setProcess)
    void setProcess(const QString &process);
    QString process() const;

    Q_PROPERTY(QString chdir READ chdir WRITE setChdir)
    void setChdir(const QString &chdir);
    QString chdir() const;

    Q_PROPERTY(QString http_socket READ httpSocket WRITE setHttpSocket)
    void setHttpSocket(const QString &httpSocket);
    QString httpSocket() const;

    Q_PROPERTY(QString chdir2 READ chdir2 WRITE setChdir2)
    void setChdir2(const QString &chdir2);
    QString chdir2() const;

    Q_PROPERTY(QString ini READ ini WRITE setIni)
    void setIni(const QString &ini);
    QString ini() const;

    Q_PROPERTY(QString static_map READ staticMap WRITE setStaticMap)
    void setStaticMap(const QString &staticMap);
    QString staticMap() const;

    Q_PROPERTY(QString static_map2 READ staticMap2 WRITE setStaticMap2)
    void setStaticMap2(const QString &staticMap);
    QString staticMap2() const;

    Q_PROPERTY(bool master READ master WRITE setMaster)
    void setMaster(bool enable);
    bool master() const;

    Q_PROPERTY(qint64 buffer_size READ bufferSize WRITE setBufferSize)
    void setBufferSize(qint64 size);
    int bufferSize() const;

    Q_PROPERTY(qint64 post_buffering READ postBuffering WRITE setPostBuffering)
    void setPostBuffering(qint64 size);
    qint64 postBuffering() const;

    Q_PROPERTY(qint64 post_buffering_bufsize READ postBufferingBufsize WRITE setPostBufferingBufsize)
    void setPostBufferingBufsize(qint64 size);
    qint64 postBufferingBufsize() const;

    Q_PROPERTY(bool tcpNodelay READ tcpNodelay WRITE setTcpNodelay)
    void setTcpNodelay(bool enable);
    bool tcpNodelay() const;

    Q_PROPERTY(bool soKeepalive READ soKeepalive WRITE setSoKeepalive)
    void setSoKeepalive(bool enable);
    bool soKeepalive() const;

    Q_PROPERTY(int socketSndbuf READ socketSndbuf WRITE setSocketSndbuf)
    void setSocketSndbuf(int value);
    int socketSndbuf() const;

    Q_PROPERTY(int socketRcvbuf READ socketRcvbuf WRITE setSocketRcvbuf)
    void setSocketRcvbuf(int value);
    int socketRcvbuf() const;

protected:
    WSGIPrivate *d_ptr;
};

}

#endif // WSGI_H
