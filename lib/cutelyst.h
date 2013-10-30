#ifndef CUTELYST_H
#define CUTELYST_H

#include <QObject>

#include "cutelystrequest.h"
#include "cutelystresponse.h"

class Cutelyst : public QObject
{
    Q_OBJECT
public:
    explicit Cutelyst(QObject *parent = 0);

    QString uriPrefix() const;
    CutelystRequest request() const;
    CutelystResponse* response() const;

    QVariantHash* stash();
};

#endif // CUTELYST_H
