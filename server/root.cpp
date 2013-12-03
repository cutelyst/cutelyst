#include "root.h"

#include <cutelyst.h>
#include <cutelystresponse.h>

#include <QDebug>

Root::Root()
{
    qDebug() << Q_FUNC_INFO;
}

void Root::hugeNameQuiteLong(Cutelyst *c, const QString &nome, Local)
{

}

void Root::begin(Cutelyst *c, const QString &name, Path)
{
    qDebug() << Q_FUNC_INFO << name << sender();
}

void Root::users(Cutelyst *c, const QString &name, const QString &age, Args, CutelystController::Local)
{
    qDebug() << Q_FUNC_INFO << name << age;
    c->response()->redirect(QLatin1String("http://www.uol.com.br"));
    c->detach();
}

void Root::admin(Cutelyst *c, const QString &name, const QString &age, CutelystController::Global)
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
    c->response()->body() = data;
}

void Root::Begin(Cutelyst *c)
{
    qDebug() << "*** Root::Begin()" << sender();
}

bool Root::Auto(Cutelyst *c)
{
    qDebug() << "*** Root::Auto()";
    return true;
}

void Root::End(Cutelyst *c)
{
    qDebug() << "*** Root::End()";
}
