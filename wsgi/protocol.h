#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <QObject>

#include "cuteengine.h"

class Protocol : public QObject
{
    Q_OBJECT
public:
    explicit Protocol(QObject *parent = 0);

    virtual void readyRead() = 0;
};

#endif // PROTOCOL_H
