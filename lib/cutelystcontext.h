#ifndef CUTELYST_H
#define CUTELYST_H

#include <QObject>

#include "cutelystrequest.h"
#include "cutelystresponse.h"

class CutelystContext : public QObject
{
    Q_OBJECT
public:
    explicit CutelystContext(QObject *parent = 0);

    QStringList args() const;
    QString uriPrefix() const;
    CutelystRequest request() const;
    CutelystResponse* response() const;

    QVariantHash* stash();
};

#endif // CUTELYST_H
