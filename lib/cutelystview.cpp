#include "cutelystview.h"
#include "context.h"
#include "cutelystresponse.h"
#include "cutelystrequest.h"

using namespace Cutelyst;

CutelystView::CutelystView(QObject *parent) :
    QObject(parent)
{
}

bool CutelystView::process(Context *ctx)
{
    if (ctx->res()->contentType().isEmpty()) {
        ctx->res()->setContentType(QLatin1String("text/html; charset=utf-8"));
    }

    if (ctx->req()->method() == "HEAD") {
        return true;
    }

    if (!ctx->res()->body().isNull()) {
        return true;
    }

    if (ctx->res()->status() == 204 ||
            (ctx->res()->status() >= 300 &&
             ctx->res()->status() < 400)) {
            return true;
    }

    return render(ctx);
}

bool CutelystView::render(Context *ctx)
{
    Q_UNUSED(ctx)
    qFatal("directly inherits from Catalyst::View. You need to\n"
           " inherit from a subclass like Cutelyst::View::ClearSilver instead.\n");
    return false;
}
