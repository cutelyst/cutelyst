#ifndef CUTELYSTREQUEST_P_H
#define CUTELYSTREQUEST_P_H

#include "cutelystrequest.h"

#include <QStringList>
#include <QHostAddress>

class CutelystEngine;
class CutelystRequestPrivate
{
public:
    QString method;
    QStringList args;
    QString protocol;
    QHash<QString, QString> cookies;
    QHash<QString, QString> headers;
    CutelystEngine *engine;
};

#endif // CUTELYSTREQUEST_P_H
