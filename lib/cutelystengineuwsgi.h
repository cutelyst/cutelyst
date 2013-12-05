#ifndef CUTELYSTENGINEUWSGI_H
#define CUTELYSTENGINEUWSGI_H

#include "cutelystengine.h"

namespace Cutelyst {

class CutelystEngineUwsgi : public Engine
{
    Q_OBJECT
public:
    explicit CutelystEngineUwsgi(int socket, Dispatcher *dispatcher, QObject *parent = 0);

};

}

#endif // CUTELYSTENGINEUWSGI_H
