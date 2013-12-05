#ifndef CUTELYSTVIEW_H
#define CUTELYSTVIEW_H

#include <QObject>

class Cutelyst;
class CutelystView : public QObject
{
    Q_OBJECT
public:
    explicit CutelystView(QObject *parent = 0);

    bool process(Cutelyst *c);

    /**
     * All subclasses must reimplement this to
     * do it's rendering.
     */
    virtual bool render(Cutelyst *c);
};

#endif // CUTELYSTVIEW_H
