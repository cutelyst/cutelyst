/*
 * Copyright (C) 2014-2015 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>
#include <Cutelyst/context.h>

namespace Cutelyst {

class StaticSimplePrivate;
class CUTELYST_PLUGIN_STATICSIMPLE_EXPORT StaticSimple : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StaticSimple)
public:
    /**
     * Constructs a new static simple object with the given parent.
     */
    StaticSimple(Application *parent);
    virtual ~StaticSimple();

    /**
     * Sets a list of directories in which to search for your static files.
     * The directories will be searched in order and will return the first file found.
     * Note that your root directory is not automatically added to the search path when
     * you specify an include_path.
     */
    void setIncludePaths(const QStringList &paths);

    /**
     * Sets a list of top-level directories beneath your 'root' directory that should
     * always be served in static mode.
     */
    void setDirs(const QStringList &dirs);

    /**
     * Reimplemented from Plugin::setup().
     */
    virtual bool setup(Application *app);

protected:
    StaticSimplePrivate *d_ptr;

private:
    void beforePrepareAction(Context *c, bool *skipMethod);
    bool locateStaticFile(Context *c, const QString &relPath);
};

}

#endif // CPSTATICSIMPLE_H
