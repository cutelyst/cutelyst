/*
 * SPDX-FileCopyrightText: (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "staticmap.h"

#include "socket.h"

#include <Cutelyst/Application>
#include <Cutelyst/Request>
#include <Cutelyst/Response>

#include <QDir>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_SM, "cutelyst.server.staticmap", QtWarningMsg)

using namespace Cutelyst;

StaticMap::StaticMap(Cutelyst::Application *parent)
    : Plugin(parent)
{
}

bool StaticMap::setup(Cutelyst::Application *app)
{
    connect(app, &Cutelyst::Application::beforePrepareAction, this, &StaticMap::beforePrepareAction);
    return true;
}

void StaticMap::addStaticMap(const QString &mountPoint, const QString &path, bool append)
{
    QString mp = mountPoint;
    if (!mp.startsWith(QLatin1Char('/'))) {
        mp.prepend(QLatin1Char('/'));
    }

    qCInfo(CUTELYST_SM) << "added mapping for" << mp << "=>" << path;

    m_staticMaps.push_back({mp, path, append});
    std::sort(m_staticMaps.begin(), m_staticMaps.end(), [](const MountPoint &a, const MountPoint &b) -> bool {
        return a.mountPoint.size() < b.mountPoint.size();
    });
}

void StaticMap::beforePrepareAction(Cutelyst::Context *c, bool *skipMethod)
{
    if (*skipMethod) {
        return;
    }

    const QString path = QLatin1Char('/') + c->req()->path();
    for (const MountPoint &mp : m_staticMaps) {
        if (path.startsWith(mp.mountPoint)) {
            if (tryToServeFile(c, mp, path)) {
                *skipMethod = true;
                break;
            }
        }
    }
}

bool StaticMap::tryToServeFile(Cutelyst::Context *c, const MountPoint &mp, const QString &path)
{
    QString localPath = path;
    if (!mp.append) {
        localPath = path.mid(mp.mountPoint.size());
        while (localPath.startsWith(QLatin1Char('/'))) {
            localPath.remove(0, 1);
        }
    }

    QDir dir(mp.path);
    QString absFilePath = dir.absoluteFilePath(localPath);
    if (!QFile::exists(absFilePath)) {
        return false;
    }

    return serveFile(c, absFilePath);
}

bool StaticMap::serveFile(Cutelyst::Context *c, const QString &filename)
{
    auto res                        = c->response();
    const QDateTime currentDateTime = QFileInfo(filename).lastModified();
    if (!c->request()->headers().ifModifiedSince(currentDateTime)) {
        res->setStatus(Response::NotModified);
        return true;
    }

    auto file = new QFile(filename);
    if (file->open(QFile::ReadOnly)) {
        qCDebug(CUTELYST_SM) << "Serving" << filename;
        Headers &headers = res->headers();

        // set our open file
        res->setBody(file);

        // use the extension to match to be faster
        QMimeType mimeType = m_db.mimeTypeForFile(filename, QMimeDatabase::MatchExtension);
        if (mimeType.isValid()) {
            headers.setContentType(mimeType.name());
        }
        headers.setContentLength(file->size());

        headers.setLastModified(currentDateTime);
        // Tell Firefox & friends its OK to cache, even over SSL
        headers.setHeader(QStringLiteral("cache_control"), QStringLiteral("public"));

        return true;
    }

    qCWarning(CUTELYST_SM) << "Could not serve" << filename << file->errorString();
    delete file;
    return false;
}

#include "moc_staticmap.cpp"
