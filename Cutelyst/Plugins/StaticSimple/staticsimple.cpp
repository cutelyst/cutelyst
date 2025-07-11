/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "application.h"
#include "context.h"
#include "request.h"
#include "response.h"
#include "staticsimple_p.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QLoggingCategory>
#include <QMimeDatabase>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

Q_LOGGING_CATEGORY(C_STATICSIMPLE, "cutelyst.plugin.staticsimple", QtWarningMsg)

StaticSimple::StaticSimple(Application *parent)
    : Plugin(parent)
    , d_ptr(new StaticSimplePrivate)
{
    Q_D(StaticSimple);
    d->includePaths.append(parent->config(u"root"_s).toString());
}

StaticSimple::~StaticSimple()
{
    delete d_ptr;
}

void StaticSimple::setIncludePaths(const QStringList &paths)
{
    Q_D(StaticSimple);
    d->includePaths.clear();
    for (const QString &path : paths) {
        d->includePaths.append(QDir(path));
    }
}

void StaticSimple::setDirs(const QStringList &dirs)
{
    Q_D(StaticSimple);
    d->dirs = dirs;
}

void StaticSimple::setServeDirsOnly(bool dirsOnly)
{
    Q_D(StaticSimple);
    d->serveDirsOnly = dirsOnly;
}

bool StaticSimple::setup(Cutelyst::Application *app)
{
    connect(app, &Application::beforePrepareAction, this, &StaticSimple::beforePrepareAction);
    return true;
}

void StaticSimple::beforePrepareAction(Context *c, bool *skipMethod) const
{
    Q_D(const StaticSimple);

    if (*skipMethod) {
        return;
    }

    const QString path          = c->req()->path().mid(1);
    const QRegularExpression re = d->re; // Thread-safe

    bool found = std::ranges::any_of(d->dirs, [&](const QString &dir) {
        if (path.startsWith(dir)) {
            if (!locateStaticFile(c, path)) {
                Response *res = c->response();
                res->setStatus(Response::NotFound);
                res->setContentType("text/html"_ba);
                res->setBody("File not found: " + path.toUtf8());
            }
            return true;
        }
        return false;
    });

    if (found) {
        *skipMethod = true;
        return;
    }

    if (d->serveDirsOnly) {
        return;
    }

    QRegularExpressionMatch match = re.match(path);
    if (match.hasMatch() && locateStaticFile(c, path)) {
        *skipMethod = true;
    }
}

bool StaticSimple::locateStaticFile(Context *c, const QString &relPath) const
{
    Q_D(const StaticSimple);

    for (const QDir &includePath : d->includePaths) {
        QString path = includePath.absoluteFilePath(relPath);
        QFileInfo fileInfo(path);
        if (fileInfo.exists()) {
            Response *res                   = c->res();
            const QDateTime currentDateTime = fileInfo.lastModified();
            if (!c->req()->headers().ifModifiedSince(currentDateTime)) {
                res->setStatus(Response::NotModified);
                return true;
            }

            // Response::setBody() will take the ownership
            // NOLINTNEXTLINE(cppcoreguidelines-owning-memory)
            auto file = new QFile(path);
            if (file->open(QFile::ReadOnly)) {
                qCDebug(C_STATICSIMPLE) << "Serving" << path;
                Headers &headers = res->headers();

                // set our open file
                res->setBody(file);

                static QMimeDatabase db;
                // use the extension to match to be faster
                QMimeType mimeType = db.mimeTypeForFile(path, QMimeDatabase::MatchExtension);
                if (mimeType.isValid()) {
                    headers.setContentType(mimeType.name().toLatin1());
                }

                headers.setLastModified(currentDateTime);
                // Tell Firefox & friends its OK to cache, even over SSL
                headers.setHeader("Cache-Control"_ba, "public"_ba);

                return true;
            }

            qCWarning(C_STATICSIMPLE) << "Could not serve" << path << file->errorString();
            return false;
        }
    }

    qCWarning(C_STATICSIMPLE) << "File not found" << relPath;
    return false;
}

#include "moc_staticsimple.cpp"
