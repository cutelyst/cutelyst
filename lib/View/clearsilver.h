#ifndef CLEARSILVER_H
#define CLEARSILVER_H

#include <QObject>

#include "cutelystview.h"

class ClearSilverPrivate;
class ClearSilver : public CutelystView
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClearSilver)
public:
    explicit ClearSilver(QObject *parent = 0);
    ~ClearSilver();

    void setRootDir(const QString &path);

    bool process(Cutelyst *c);

protected:
    ClearSilverPrivate *d_ptr;
};

#endif // CLEARSILVER_H
