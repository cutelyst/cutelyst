#include "users.h"

#include <QDebug>

Users::Users()
{

}

void Users::list(Cutelyst *c, const QString &name, const QString &age, CutelystController::Args, CutelystController::Local)
{
    qDebug() << Q_FUNC_INFO << name << age;
//    qDebug() << "Cookies" << c->req()->cookies();

    QNetworkCookie cookie;
    cookie.setName("bar");
    cookie.setValue("lolololo");
    cookie.setPath("/");
    c->response()->addCookie(cookie);
}

void Users::Begin(Cutelyst *c)
{
    qDebug() << "*** Users::Begin()";
}

bool Users::Auto(Cutelyst *c)
{
    qDebug() << "*** Users::Auto()";
    return true;
}

//void Users::End(Cutelyst *c)
//{
//    qDebug() << "*** Users::End()";
//}
