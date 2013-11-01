#include "root.h"

#include <QDebug>

Root::Root()
{
    qDebug() << Q_FUNC_INFO;
}

void Root::begin(CutelystContext *c, const QString &nome)
{
    qDebug() << Q_FUNC_INFO << nome;
}

void Root::begin(CutelystContext *c, const QString &nome, const QString &idade)
{
    qDebug() << Q_FUNC_INFO << nome << idade;
}
