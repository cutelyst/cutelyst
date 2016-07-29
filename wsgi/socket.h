#ifndef SOCKET_H
#define SOCKET_H

#include <QTcpSocket>
#include <QHostAddress>
#include <Cutelyst/Headers>

class QIODevice;

namespace CWSGI {

class CWsgiEngine;
class Socket
{
public:
    Socket();
    virtual ~Socket();

    Cutelyst::Headers headers;
    QString serverAddress;
    QString method;
    QString path;
    QByteArray query;
    QString protocol;
    QByteArray buffer;
    quint64 start;
    CWsgiEngine *engine;
    char *buf;
    int buf_size = 0;
    int connState = 0;//
    int beginLine = 0;
    int last = 0;
};

class TcpSocket : public QTcpSocket, public Socket
{
    Q_OBJECT
public:
    explicit TcpSocket(QObject *parent = 0);

Q_SIGNALS:
    void readyRead();
};

}

#endif // SOCKET_H
