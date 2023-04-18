/*
 * SPDX-FileCopyrightText: (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATICMAP_H
#define STATICMAP_H

#include <Cutelyst/Context>
#include <Cutelyst/Plugin>
#include <vector>

#include <QMimeDatabase>
#include <QString>

struct MountPoint {
    QString mountPoint;
    QString path;
    bool append;
};

class Socket;
class StaticMap final : public Cutelyst::Plugin
{
    Q_OBJECT
public:
    StaticMap(Cutelyst::Application *parent);

    virtual bool setup(Cutelyst::Application *app) override;

    void addStaticMap(const QString &mountPoint, const QString &path, bool append);

private:
    void beforePrepareAction(Cutelyst::Context *c, bool *skipMethod);

    bool tryToServeFile(Cutelyst::Context *c, const MountPoint &mp, const QString &path);

    bool serveFile(Cutelyst::Context *c, const QString &filename);

    QMimeDatabase m_db;
    std::vector<MountPoint> m_staticMaps;
};

#endif // STATICMAP_H
