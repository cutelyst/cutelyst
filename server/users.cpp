#include "users.h"

#include <QDebug>

Users::Users()
{

}

void Users::list(const QString &name, const QString &age, CutelystController::Args, CutelystController::Local)
{
    qDebug() << Q_FUNC_INFO << name << age;
//    qDebug() << "Cookies" << c->req()->cookies();

    QNetworkCookie cookie;
    cookie.setName("bar");
    cookie.setValue("lolololo");
    cookie.setPath("/");
    c->response()->addCookie(cookie);
}

void Users::Begin()
{
    qDebug() << "*** Users::Begin()";
}

void Users::Auto()
{
    qDebug() << "*** Users::Auto()";
}

//void Users::End()
//{
//    qDebug() << "*** Users::End()";
//}
