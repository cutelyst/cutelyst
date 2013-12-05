#include "root.h"

#include <context.h>
#include <cutelystresponse.h>

#include <QDebug>

Root::Root()
{
    qDebug() << Q_FUNC_INFO;
}

void Root::hugeNameQuiteLong(Context *ctx, const QString &nome, Local)
{

}

void Root::begin(Context *ctx, const QString &name, Path)
{
    qDebug() << Q_FUNC_INFO << name << sender();
}

void Root::users(Context *ctx, const QString &name, const QString &age, Args, CutelystController::Local)
{
    qDebug() << Q_FUNC_INFO << name << age;
    ctx->response()->redirect(QLatin1String("http://www.uol.com.br"));
    ctx->detach();
}

void Root::admin(Context *ctx, const QString &name, const QString &age, CutelystController::Global)
{
    qDebug() << Q_FUNC_INFO << name << age;
    QByteArray data;
    data = "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "    <head>"
            "        <meta charset=\"utf-8\">"
            "        <title>Hello World</title>"
            "    </head>"
            "    <body>"
            "        <h1>Hello World</h1>"
            "        <p>"
            "            Jamie estava aqui."
            "        </p>"
            "    </body>"
            "</html>";
    ctx->response()->body() = data;
}

void Root::Begin(Context *ctx)
{
    qDebug() << "*** Root::Begin()" << sender();
}

bool Root::Auto(Context *ctx)
{
    qDebug() << "*** Root::Auto()";
    return true;
}

void Root::End(Context *ctx)
{
    qDebug() << "*** Root::End()";
}
