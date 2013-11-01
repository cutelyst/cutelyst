#ifndef CUTELYSTCONTROLLER_H
#define CUTELYSTCONTROLLER_H

#include <QObject>

#include "cutelystcontext.h"

class CutelystController : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("Controller", "")
public:
    Q_INVOKABLE explicit CutelystController(QObject *parent = 0);
    ~CutelystController();

};

Q_DECLARE_METATYPE(CutelystController*)

#endif // CUTELYSTCONTROLLER_H
