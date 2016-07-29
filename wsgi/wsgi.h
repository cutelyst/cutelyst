#ifndef WSGI_H
#define WSGI_H

#include <QObject>
#include <QTcpServer>
#include <Cutelyst/Engine>
#include <Cutelyst/Application>

#if defined(cutelyst_wsgi_qt5_EXPORTS)
#  define CUTELYST_WSGI_EXPORT Q_DECL_EXPORT
#else
#  define CUTELYST_WSGI_EXPORT Q_DECL_IMPORT
#endif

class QIODevice;

namespace CWSGI {

class Protocol;
class CuteEngine;
class CUTELYST_WSGI_EXPORT WSGI : public QObject
{
    Q_OBJECT
public:
    explicit WSGI(QObject *parent = 0);

    bool listenTcp(const QString &line);

    bool listenSocket(const QString &address);

    bool loadApplication();

    Q_PROPERTY(QString application READ application WRITE setApplication)
    void setApplication(const QString &application);
    QString application() const;

    Q_PROPERTY(int threads READ threads WRITE setThreads)
    void setThreads(int threads);
    int threads() const;

    Q_PROPERTY(QString chdir READ chdir WRITE setChdir)
    void setChdir(const QString &chdir);
    QString chdir() const;

    Q_PROPERTY(QString httpSocket READ httpSocket WRITE setHttpSocket)
    void setHttpSocket(const QString &httpSocket);
    QString httpSocket() const;

    Q_PROPERTY(QString chdir2 READ chdir2 WRITE setChdir2)
    void setChdir2(const QString &chdir2);
    QString chdir2() const;

    Q_PROPERTY(QString ini READ ini WRITE setIni)
    void setIni(const QString &ini);
    QString ini() const;

private:
    CuteEngine *createEngine(Cutelyst::Application *app, int core);

    bool loadConfig();

    QVector<QTcpServer *> m_sockets;
    QVector<Cutelyst::Engine *> m_engines;
    CuteEngine *m_engine;

    QString m_application;
    QString m_chdir;
    QString m_chdir2;
    QString m_ini;
    int m_threads = 0;

};

}

#endif // WSGI_H
