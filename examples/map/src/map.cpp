#include "map.h"

#include "root.h"

using namespace Cutelyst;

Map::Map(QObject *parent) : Application(parent)
{
}

Map::~Map()
{
}

bool Map::init()
{
    new Root(this);

    return true;
}

