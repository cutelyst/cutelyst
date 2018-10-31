#include "server_01.h"

#include "root.h"

using namespace Cutelyst;

server_01::server_01(QObject *parent) : Application(parent)
{
}

server_01::~server_01()
{
}

bool server_01::init()
{
    new Root(this);

    return true;
}

