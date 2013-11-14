#include "users.h"

#include <QDebug>

Users::Users()
{

}

void Users::list(const QString &name, const QString &age, CutelystController::Args, CutelystController::Local)
{
    qDebug() << Q_FUNC_INFO << name << age;
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
