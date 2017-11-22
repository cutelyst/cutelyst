/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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
#ifndef STATICCOMPRESSED_H
#define STATICCOMPRESSED_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Plugin>

namespace Cutelyst {

class StaticCompressedPrivate;

class CUTELYST_PLUGIN_STATICCOMPRESSED_EXPORT StaticCompressed : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StaticCompressed)
public:
    /**
     * Constructs a new StaticCompressed object with the given @a parent.
     */
    explicit StaticCompressed(Application *parent);

    /**
     * Deconstructs the StaticCompressed object.
     */
    virtual ~StaticCompressed();

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
     * Configures the plugin by reading the @c Cutelyst_StaticCompressed_Plugin section
     * from the Cutelyst application configuration file and connects to the
     * Application::beforePrepareAction() signal.
     */
    virtual bool setup(Application *app) override;

protected:
    QScopedPointer<StaticCompressedPrivate> d_ptr;
};

}

#endif // STATICCOMPRESSED_H
