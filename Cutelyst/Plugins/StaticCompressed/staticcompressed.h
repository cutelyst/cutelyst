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

class Context;
class StaticCompressedPrivate;

class CUTELYST_PLUGIN_STATICCOMPRESSED_EXPORT StaticCompressed : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StaticCompressed)
public:
    explicit StaticCompressed(Application *parent);

    virtual ~StaticCompressed();

    void setIncludePaths(const QStringList &paths);

    void setDirs(const QStringList &dirs);

    void setCacheDir(const QString &cacheDir);

    virtual bool setup(Application *app) override;

protected:
    QScopedPointer<StaticCompressedPrivate> d_ptr;

private:
    void beforePrepareAction(Context *c, bool *skipMethod);
};

}

#endif // STATICCOMPRESSED_H
