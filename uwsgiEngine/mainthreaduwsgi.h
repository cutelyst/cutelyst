#ifndef MAINTHREADUWSGI_H
#define MAINTHREADUWSGI_H

#include <QObject>

#include "engineuwsgi.h"

class MainThreadUWSGI : public QObject
{
    Q_OBJECT
public:
    explicit MainThreadUWSGI(QObject *parent = 0);

    void requestFinished(struct wsgi_request *wsgi_req);
};

#endif // MAINTHREADUWSGI_H
