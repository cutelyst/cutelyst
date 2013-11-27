#include "staticsimple.h"

#include "cutelystapplication.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"
#include "cutelyst.h"

#include <QRegularExpression>
#include <QStringBuilder>
#include <QFile>
#include <QDir>
#include <QDebug>

using namespace CutelystPlugin;

StaticSimple::StaticSimple(QObject *parent) :
    Plugin(parent),
    m_rootDir(QDir::currentPath())
{
}

void StaticSimple::setRootDir(const QString &path)
{
    m_rootDir = path;
}

bool StaticSimple::setup(CutelystApplication *app)
{
    qDebug() << Q_FUNC_INFO << m_rootDir;
    connect(app, &CutelystApplication::beforePrepareAction,
            this, &StaticSimple::beforePrepareAction);
}

void StaticSimple::beforePrepareAction(Cutelyst *c, bool *skipMethod)
{
    if (*skipMethod) {
        return;
    }

    QString path = c->req()->path();
    QRegularExpression re("\\.\\S+$");
    QRegularExpressionMatch match = re.match(path);
    if (match.hasMatch()) {
        qDebug() << Q_FUNC_INFO << path << sender();
        if (locateStaticFile(c, path)) {
            *skipMethod = true;
        }
    }
}


bool StaticSimple::locateStaticFile(Cutelyst *c, QString &path)
{
    path = m_rootDir % path;
    QFile file(path);
    qDebug() << Q_FUNC_INFO << "tying to serve" << path << file.fileName();
    if (file.exists() && file.open(QFile::ReadOnly)) {
        c->response()->setBody(file.readAll());
        return true;
    } else {
        qDebug() << Q_FUNC_INFO << "not serving" << file.errorString();
    }
    return false;
}
