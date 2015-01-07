#include "staticsimple_p.h"

#include "application.h"
#include "request.h"
#include "response.h"
#include "context.h"

#include <QStringBuilder>
#include <QMimeDatabase>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_STATICSIMPLE, "cutelyst.plugin.staticsimple")

StaticSimple::StaticSimple(QObject *parent) :
    Plugin(parent),
    d_ptr(new StaticSimplePrivate)
{
    Q_D(StaticSimple);
    d->includePaths.append(QDir::currentPath());
}

StaticSimple::~StaticSimple()
{
    delete d_ptr;
}

void StaticSimple::setIncludePaths(const QStringList &paths)
{
    Q_D(StaticSimple);
    d->includePaths.clear();
    Q_FOREACH (const QString &path, paths) {
        d->includePaths.append(QDir(path));
    }
}

void StaticSimple::setDirs(const QStringList &dirs)
{
    Q_D(StaticSimple);
    d->dirs = dirs;
}

bool StaticSimple::setup(Context *ctx)
{
    connect(ctx, &Context::beforePrepareAction,
            this, &StaticSimple::beforePrepareAction);
    return true;
}

bool StaticSimple::isApplicationPlugin() const
{
    return true;
}

void StaticSimple::beforePrepareAction(bool *skipMethod)
{
    Q_D(StaticSimple);

    Context *ctx = static_cast<Context *>(sender());
    if (*skipMethod || !ctx) {
        return;
    }

    QString path = ctx->req()->path();
    QRegularExpression re = d->re; // Thread-safe
    QRegularExpressionMatch match = re.match(path);
    if (match.hasMatch() && locateStaticFile(ctx, path)) {
        *skipMethod = true;
    }
}


bool StaticSimple::locateStaticFile(Context *ctx, const QString &relPath)
{
    Q_D(StaticSimple);

    Q_FOREACH (const QDir &includePath, d->includePaths) {
        QString path = includePath.absoluteFilePath(relPath);
        QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            Response *res = ctx->res();
            QDateTime utc = fileInfo.lastModified().toTimeSpec(Qt::UTC);
            QString lastModified = utc.toString(QStringLiteral("ddd, dd MMM yyyy hh:mm:ss")) % QLatin1String(" GMT");
            if (lastModified == ctx->req()->headers().ifModifiedSince()) {
                res->setStatus(Response::NotModified);
                return true;
            }

            QFile *file = new QFile(path);
            if (file->open(QFile::ReadOnly)) {
                qCDebug(C_STATICSIMPLE) << "Serving" << path;
                Headers &headers = res->headers();

                // set our open file
                res->setBody(file);

                QMimeDatabase db;
                // use the extension to match to be faster
                QMimeType mimeType = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
                if (mimeType.isValid()) {
                    QString contentType = mimeType.name() % QLatin1String("; charset=utf-8");
                    headers.setContentType(contentType.toLatin1());
                }

                headers.setLastModified(lastModified.toLocal8Bit());
                headers.insert(QStringLiteral("Cache-Control"), QStringLiteral("public"));

                return true;
            }

            qCWarning(C_STATICSIMPLE) << "Could not serve" << path << file->errorString();
            return false;
        }
    }

    qCWarning(C_STATICSIMPLE) << "File not found" << relPath;
    return false;
}
