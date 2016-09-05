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
#include <QVector>
#include <QProcess>

#if defined(cutelyst_wsgi_qt5_EXPORTS)
#  define CUTELYST_WSGI_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_WSGI_EXPORT Q_DECL_IMPORT
#endif

class QTcpServer;
class QCoreApplication;

namespace Cutelyst {
class Application;
class Engine;
}

namespace CWSGI {

class Protocol;
class CWsgiEngine;
class CUTELYST_WSGI_EXPORT WSGI : public QObject
{
    Q_OBJECT
public:
    explicit WSGI(QObject *parent = 0);
    virtual ~WSGI();

    int load(const QCoreApplication &app);

    Q_PROPERTY(QString application READ application WRITE setApplication)
    void setApplication(const QString &application);
    QString application() const;

    Q_PROPERTY(int threads READ threads WRITE setThreads)
    void setThreads(int threads);
    int threads() const;

    Q_PROPERTY(int process READ process WRITE setProcess)
    void setProcess(int process);
    int process() const;

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

Q_SIGNALS:
    void forked();

private:
    bool listenTcp(const QString &line);
    bool listenSocket(const QString &address);
    void proc();
    int parseCommandLine(const QCoreApplication &app);
    int setupApplication();
    void childFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void engineInitted();

    CWsgiEngine *createEngine(Cutelyst::Application *app, int core);

    bool loadConfig();

    QVector<QTcpServer *> m_sockets;
    QVector<Cutelyst::Engine *> m_engines;
    CWsgiEngine *m_engine;

    QString m_application;
    QString m_chdir;
    QString m_chdir2;
    QString m_ini;
    int m_bufferSize = 4096;
    qint64 m_postBuffering = -1;
    qint64 m_postBufferingBufsize = 4096;
    int m_enginesInitted = 1;
    int m_threads = 0;
    int m_process = 0;
    bool m_master = false;

};

}

#endif // WSGI_H
