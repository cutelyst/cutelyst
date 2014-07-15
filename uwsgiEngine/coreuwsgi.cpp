#include "coreuwsgi.h"
#include "engineuwsgi.h"

#include <QMutex>
#include <QWaitCondition>

#define UWSGI_QT_EVENT_NULL	0
#define UWSGI_QT_EVENT_REQUEST	1
#define UWSGI_QT_EVENT_SIGNAL	2

QWaitCondition QueueReady;
QWaitCondition QueueFree;
QMutex mutex;

int qt_uwsgi_req_fd = 0;

CoreUWSGI::CoreUWSGI(Cutelyst::Application *app, int coreId) :
    m_app(app),
    m_coreId(coreId)
{

}

void CoreUWSGI::run()
{
    // TODO is this really needed?
    uwsgi_setup_thread_req(m_coreId, &uwsgi.workers[uwsgi.mywid].cores[m_coreId].req);

    while (uwsgi.workers[uwsgi.mywid].manage_next_request) {
        mutex.lock();
        if (qt_uwsgi_req_fd == 0) {
            QueueReady.wait(&mutex);
        }

        // this is required when using wakeAll()
        if (qt_uwsgi_req_fd == 0) {
            mutex.unlock();
            continue;
        }

//        int fd = qt_uwsgi_req_fd;

        qt_uwsgi_req_fd = 0;

        QueueFree.wakeOne();

        mutex.unlock();

        // run the handler
//        uch->handle_request(fd);
    }
}
