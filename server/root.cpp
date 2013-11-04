#include "root.h"

#include <cutelystcontext.h>

#include <QDebug>

Root::Root()
{
    qDebug() << Q_FUNC_INFO;
}

void Root::begin(const QString &nome)
{
    qDebug() << Q_FUNC_INFO << nome;
}

void Root::begin(const QString &nome, const QString &idade, CaptureArgs)
{
    qDebug() << Q_FUNC_INFO << nome << idade;
}
