#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpServer>

#include "protocol.h"
#include "socket.h"

namespace CWSGI {

class CWsgiEngine;
class TcpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit TcpServer(QObject *parent = 0);

    virtual void incomingConnection(qintptr handle);

    Protocol *m_proto;
    CWsgiEngine *m_engine;

    QVector<TcpSocket *> m_socks;
};

}

#endif // TCPSERVER_H
