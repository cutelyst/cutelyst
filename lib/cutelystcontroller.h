#ifndef CUTELYSTCONTROLLER_H
#define CUTELYSTCONTROLLER_H

#include <QObject>

#include "cutelyst.h"

class CutelystController : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("Controller", "")
public:
    explicit CutelystController(QObject *parent = 0);
    ~CutelystController();

private:
};

Q_DECLARE_METATYPE(CutelystController*)

#endif // CUTELYSTCONTROLLER_H
