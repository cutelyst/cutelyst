#include "cstaticsimple.h"

#include "cutelystapplication.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"
#include "cutelyst.h"

#include <QDebug>
#include <QFile>

CPStaticSimple::CPStaticSimple(QObject *parent) :
    CutelystPlugin(parent)
{
}

bool CPStaticSimple::setup(CutelystApplication *app)
{
    connect(app, &CutelystApplication::beforePrepareAction,
            this, &CPStaticSimple::beforePrepareAction);
}

void CPStaticSimple::beforePrepareAction(Cutelyst *c, bool *skipMethod)
{
    if (*skipMethod) {
        return;
    }

    QString path = c->req()->path();
    qDebug() << Q_FUNC_INFO << path << sender();
    if (path.startsWith(QLatin1String("/static"))) {
        path.remove(0, 8);
        QFile file(path);
        qDebug() << Q_FUNC_INFO << "tying to serve" << path << file.fileName();
        if (file.exists() && file.open(QFile::ReadOnly)) {
            c->response()->setBody(file.readAll());
            *skipMethod = true;
        } else {
            qDebug() << Q_FUNC_INFO << "not serving" << file.errorString();
        }
    }
}
