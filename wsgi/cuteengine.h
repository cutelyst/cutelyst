#ifndef CUTEENGINE_H
#define CUTEENGINE_H

#include <QObject>
#include <Cutelyst/Engine>

#include "socket.h"

using namespace Cutelyst;

class QTcpServer;

namespace CWSGI {

class Protocol;
class CuteEngine : public Engine
{
    Q_OBJECT
public:
    explicit CuteEngine(const QVariantMap &opts, QObject *parent = 0);

    virtual int workerId() const;

    virtual int workerCore() const;

    inline void processSocket(TcpSocket *sock) {
        processRequest(sock->method,
                       sock->path,
                       sock->query,
                       sock->protocol,
                       false,
                       sock->serverAddress,
                       sock->peerAddress(),
                       sock->peerPort(),
                       QString(),
                       sock->headers,
                       sock->start,
                       0,
                       sock);
    }

    Protocol *m_proto;

    void setTcpSockets(const QVector<QTcpServer *> sockets);

    void forked();

    int m_workerId = 0;
    int m_workerCore = 0;
    Application *m_app;

protected:
    virtual bool finalizeHeaders(Context *ctx);

    virtual qint64 doWrite(Context *c, const char *data, qint64 len, void *engineData);

private:
    virtual bool init();

    void newconnectionTcp();
    void newconnectionLocalSocket();

    QVector<QTcpServer *> m_sockets;

};

}

#endif // CUTEENGINE_H
