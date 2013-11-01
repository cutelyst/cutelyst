#include "cutelystcontext.h"

#include <QStringList>

CutelystContext::CutelystContext(QObject *parent) :
    QObject(parent)
{
}

QStringList CutelystContext::args() const
{
    return QStringList() << "bla bla bla";
}
