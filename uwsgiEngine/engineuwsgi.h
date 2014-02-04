#ifndef ENGINE_UWSGI_H
#define ENGINE_UWSGI_H

#include "uwsgi.h"

#include "../Cutelyst/engine.h"

class QPluginLoader;
namespace Cutelyst {

class Dispatcher;
class Application;
class EngineUwsgi : public Engine
{
    Q_OBJECT
public:
    explicit EngineUwsgi(QObject *parent = 0);

    bool loadApplication(const QString &path);

    virtual bool init();

    virtual void finalizeHeaders(Context *ctx);
    virtual void finalizeBody(Context *ctx);

    void processRequest(wsgi_request *wsgi_req);

private:
    QPluginLoader *m_loader;
};

}

#endif // ENGINE_UWSGI_H
