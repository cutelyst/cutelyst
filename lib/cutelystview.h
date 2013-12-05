#ifndef CUTELYSTVIEW_H
#define CUTELYSTVIEW_H

#include <QObject>

namespace Cutelyst {

class Context;
class CutelystView : public QObject
{
    Q_OBJECT
public:
    explicit CutelystView(QObject *parent = 0);

    bool process(Context *ctx);

    /**
     * All subclasses must reimplement this to
     * do it's rendering.
     */
    virtual bool render(Context *ctx);
};

}

#endif // CUTELYSTVIEW_H
