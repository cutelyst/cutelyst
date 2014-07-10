#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include <QObject>

struct wsgi_request;

namespace Cutelyst {

class RequestHandler : public QObject
{
    Q_OBJECT
public:
    RequestHandler(wsgi_request *request);

    void handle_request(int);

public:
    struct wsgi_request *wsgi_req;
};

}

#endif // REQUESTHANDLER_H
