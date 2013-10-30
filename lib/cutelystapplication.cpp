#include "cutelystapplication_p.h"

#include "cutelystchildprocess.h"

#include <iostream>

#include <QStringList>
#include <QTcpSocket>
#include <QDebug>

using namespace std;

CutelystApplication::CutelystApplication(int &argc, char **argv) :
    QCoreApplication(argc, argv),
    d_ptr(new CutelystApplicationPrivate(this))
{
    Q_D(CutelystApplication);

    d->server = new QTcpServer(this);
    connect(d->server, &QTcpServer::newConnection,
            this, &CutelystApplication::onNewConnection);
}

CutelystApplication::~CutelystApplication()
{
}

bool CutelystApplication::parseArgs()
{
    Q_D(CutelystApplication);

    QStringList args = QCoreApplication::arguments();
    if (args.contains(QLatin1String("--about")) ||
            args.contains(QLatin1String("--sobre")) ||
            args.contains(QLatin1String("/sobre"))) {
        cout << QCoreApplication::applicationName().toStdString() << endl
             << QCoreApplication::applicationVersion().toStdString() << endl
             << QCoreApplication::organizationName().toStdString() << endl
             << QCoreApplication::organizationDomain().toStdString() << endl
             << "Qt: " << QT_VERSION_STR << endl;
        return false;
    }

    for (int i = 0; i < args.count(); ++i) {
        QString argument = args.at(i);
        if (argument.startsWith(QLatin1String("--port="))) {
            if (argument.length() > 7) {
                d->port = argument.mid(7).toInt();
                qDebug() << "Using custom port:" << d->port;
            }
            continue;
        }
    }

    if (d->server->listen(d->address, d->port)) {
        qDebug() << Q_FUNC_INFO << QCoreApplication::applicationPid();

        int childCount = 1;
        for (int i = 0; i < childCount; ++i) {
            bool childProcess;
            CutelystChildProcess *child = new CutelystChildProcess(childProcess, this);
            if (childProcess) {
                // We are not the parent anymore,
                // so we don't need the server class
                delete d->server;
                return true;
            } else if (child->initted()) {
                d->child << child;
            } else {
                delete child;
            }
        }
        qDebug() << "Listening on:" << d->server->serverAddress() << d->server->serverPort();
        qDebug() << "Number of child process:" << d->child.size();
    }

    return !d->child.isEmpty();
}

int CutelystApplication::printError()
{
    return 1;
}

void CutelystApplication::onNewConnection()
{
    Q_D(CutelystApplication);

    QTcpSocket *socket = d->server->nextPendingConnection();
    if (socket) {
        if (!d->child.isEmpty()) {
            if (d->child.first()->sendFD(socket->socketDescriptor())) {
//                qDebug() << "fd sent";
            }
            delete socket;
        }
    }
}


CutelystApplicationPrivate::CutelystApplicationPrivate(CutelystApplication *parent) :
    q_ptr(parent),
    port(3000),
    address(QHostAddress::Any)
{

}

CutelystApplicationPrivate::~CutelystApplicationPrivate()
{

}
