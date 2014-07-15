#ifndef COREUWSGI_H
#define COREUWSGI_H

#include <QThread>
#include <Cutelyst/Application>

class CoreUWSGI : public QThread
{
    Q_OBJECT
public:
    CoreUWSGI(Cutelyst::Application *app, int coreId);

    void run();

private:
    Cutelyst::Application *m_app;
    int m_coreId;
};

#endif // COREUWSGI_H
