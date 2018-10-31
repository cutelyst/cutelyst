#ifndef SERVER_01_H
#define SERVER_01_H

#include <Cutelyst/Application>

using namespace Cutelyst;

class server_01 : public Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "server_01")
public:
    Q_INVOKABLE explicit server_01(QObject *parent = 0);
    ~server_01();

    bool init();
};

#endif //SERVER_01_H

