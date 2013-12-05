#include "cutelystview.h"
#include "cutelyst.h"
#include "cutelystresponse.h"
#include "cutelystrequest.h"

CutelystView::CutelystView(QObject *parent) :
    QObject(parent)
{
}

bool CutelystView::process(Cutelyst *c)
{
    if (c->res()->contentType().isEmpty()) {
        c->res()->setContentType(QLatin1String("text/html; charset=utf-8"));
    }

    if (c->req()->method() == "HEAD") {
        return true;
    }

    if (!c->res()->body().isNull()) {
        return true;
    }

    if (c->res()->status() == 204 ||
            (c->res()->status() >= 300 &&
             c->res()->status() < 400)) {
            return true;
    }

    return render(c);
}

bool CutelystView::render(Cutelyst *c)
{
    Q_UNUSED(c)
    qFatal("directly inherits from Catalyst::View. You need to\n"
           " inherit from a subclass like Cutelyst::View::ClearSilver instead.\n");
    return false;
}
