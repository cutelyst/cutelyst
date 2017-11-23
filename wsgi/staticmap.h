/*
 * Copyright (C) 2016-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef STATICMAP_H
#define STATICMAP_H

#include <QString>
#include <QMimeDatabase>
#include <vector>

#include <Cutelyst/Plugin>
#include <Cutelyst/Context>

namespace CWSGI {

struct MountPoint {
    QString mountPoint;
    QString path;
    bool append;
};

class Socket;
class StaticMap : public Cutelyst::Plugin
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

}

#endif // STATICMAP_H
