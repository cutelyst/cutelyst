/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CPSTATICSIMPLE_H
#define CPSTATICSIMPLE_H

#include <Cutelyst/context.h>
#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/plugin.h>

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
    virtual ~StaticSimple() override;

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
    virtual bool setup(Application *app) override;

protected:
    StaticSimplePrivate *d_ptr;

private:
    void beforePrepareAction(Context *c, bool *skipMethod);
    bool locateStaticFile(Context *c, const QString &relPath);
};

} // namespace Cutelyst

#endif // CPSTATICSIMPLE_H
