#include "staticsimple.h"

#include "cutelystapplication.h"
#include "cutelystrequest.h"
#include "cutelystresponse.h"
#include "cutelyst.h"

#include <QRegularExpression>
#include <QStringBuilder>
#include <QMimeDatabase>
#include <QFile>
#include <QDir>
#include <QDebug>

using namespace CutelystPlugin;

StaticSimple::StaticSimple(const QString &path, QObject *parent) :
    Plugin(parent),
    m_rootDir(path)
{
    if (m_rootDir.isNull()) {
        m_rootDir = QDir::currentPath();
    }
}

void StaticSimple::setRootDir(const QString &path)
{
    m_rootDir = path;
}

bool StaticSimple::setup(CutelystApplication *app)
{
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
        if (locateStaticFile(c, path)) {
            *skipMethod = true;
        }
    }
}


bool StaticSimple::locateStaticFile(Cutelyst *c, QString &path)
{
    path = m_rootDir % path;
    QFile file(path);
    if (file.exists() && file.open(QFile::ReadOnly)) {
        c->response()->body() = file.readAll();
        QMimeDatabase db;
        // use the extension to match to be faster
        QMimeType mimeType = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
        if (mimeType.isValid()) {
            c->res()->setContentType(mimeType.name() % QLatin1String("; charset=utf-8"));
        }
        return true;
    }

    qWarning() << "Could not serve" << path << file.errorString();
    return false;
}
