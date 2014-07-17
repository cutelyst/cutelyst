#include "mainthreaduwsgi.h"

MainThreadUWSGI::MainThreadUWSGI(QObject *parent) :
    QObject(parent)
{
}

void MainThreadUWSGI::requestFinished(wsgi_request *wsgi_req)
{
    free_req_queue;
}
