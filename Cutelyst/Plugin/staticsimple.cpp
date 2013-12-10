#include "staticsimple.h"

#include "application.h"
#include "request.h"
#include "response.h"
#include "context.h"

#include <QRegularExpression>
#include <QStringBuilder>
#include <QMimeDatabase>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDebug>

using namespace Cutelyst;
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

bool StaticSimple::setup(Application *app)
{
    connect(app, &Application::beforePrepareAction,
            this, &StaticSimple::beforePrepareAction);
}

void StaticSimple::beforePrepareAction(Context *ctx, bool *skipMethod)
{
    if (*skipMethod) {
        return;
    }

    QString path = ctx->req()->path();
    QRegularExpression re("\\.\\S+$");
    QRegularExpressionMatch match = re.match(path);
    if (match.hasMatch() && locateStaticFile(ctx, path)) {
        *skipMethod = true;
    }
}


bool StaticSimple::locateStaticFile(Context *ctx, QString &path)
{
    path = m_rootDir % path;
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        QDateTime utc = fileInfo.lastModified();
        utc.setTimeSpec(Qt::UTC);
        QString lastModified;
        lastModified = utc.toString(QLatin1String("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");

        if (lastModified == ctx->req()->headers()[QLatin1String("If-Modified-Since")]) {
            ctx->res()->setStatus(Response::NotModified);
            return true;
        }

        QFile file(path);
        if (file.open(QFile::ReadOnly)) {
            qWarning() << "Serving" << path;
            ctx->response()->body() = file.readAll();
            QMimeDatabase db;
            // use the extension to match to be faster
            QMimeType mimeType = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
            if (mimeType.isValid()) {
                ctx->res()->setContentType(mimeType.name() % QLatin1String("; charset=utf-8"));
            }

            ctx->res()->headers()[QLatin1String("Last-Modified")] = lastModified;
            ctx->res()->headers()[QLatin1String("Cache-Control")] = QLatin1String("public");
            qWarning() << "File headers" << ctx->res()->headers();

            return true;
        }

        qWarning() << "Could not serve" << path << file.errorString();
        return false;
    }

    qWarning() << "File not found" << path;
    return false;
}
