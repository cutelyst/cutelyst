#ifndef COREUWSGI_H
#define COREUWSGI_H

#include <QThread>
#include <Cutelyst/Application>

#include "engineuwsgi.h"

class CoreUWSGI : public QThread
{
    Q_OBJECT
public:
    CoreUWSGI(Cutelyst::Application *app, int coreId);

    void run();

private:
    EngineUwsgi *m_engine;
    int m_coreId;
};

#endif // COREUWSGI_H
