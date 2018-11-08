#ifndef _MAP__H__
#define _MAP__H__

#include <Cutelyst/Application>

using namespace Cutelyst;

class Map : public Application
{
    Q_OBJECT
    CUTELYST_APPLICATION(IID "Map")
public:
    Q_INVOKABLE explicit Map(QObject *parent = 0);
    ~Map();

    bool init();
};

#endif //_MAP__H__

