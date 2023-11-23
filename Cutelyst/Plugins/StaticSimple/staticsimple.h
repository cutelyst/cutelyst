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
/**
 * @ingroup plugins
 * @headerfile "" <Cutelyst/Plugins/StaticSimple/StaticSimple>
 * @brief Serve static files directly from your application.
 *
 * The %StaticSimple plugin for %Cutelyst can be used to @ref servestatic "serve static files"
 * from specific directories. Optionally it only serves files where the request path starts
 * with a specific directory. Files that end with something that look like a file extension
 * will tried to be served by this plugin.
 *
 * Beside serving the file content this will also set the respective HTTP header fields
 * @c Content-Type, @c Content-Length, @c Last-Modified and @c Cache-Control=public.
 *
 * <h3>Only serve for specific request paths</h3>
 *
 * You can use setDirs() to set a list of directories/path below your web root where files should
 * always be served by this plugin. By default, the plugin also tries to server files from other
 * paths when they have a file extension when they do not start with one of these paths. You can
 * set setServeDirsOnly() to @c true (since %Cutelyst 4.0.0) to only serve files beginning with
 * these paths. Have a look at setDirs() to learn more.
 *
 * <h3>Usage example</h3>
 *
 * @code{.cpp}
 * #include <Cutelyst/Plugins/StaticSimple/StaticSimple>
 *
 * bool MyCutelystApp::init()
 * {
 *      // other initialization stuff
 *      // ...
 *
 *      // construct a new StaticSimple plugin
 *      auto stat = new StaticSimple(this);
 *      stat->setIncludePaths({"/path/to/my/static/files"});
 *
 *      // maybe more initialization stuff
 *      // ...
 * }
 * @endcode
 *
 * @logcat{plugin.staticsimple}
 *
 * @sa StaticCompressed
 * @sa @ref servestatic
 */
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
     * you specify an include path.
     */
    void setIncludePaths(const QStringList &paths);

    /**
     * Sets a list of top-level directories below your web root that should
     * always be served in static mode. Set setServeDirsOnly() to @c true to only serve
     * files in static mode (since %Cutelyst 4.0.0) if their path begins with one of the
     * listed directories. By default, this list is empty.
     *
     * If your static files for example are organized like:
     * <PRE>
     * /path/to/my/static/files/assets/css
     * /path/to/my/static/files/assets/js
     * ...
     * </PRE>
     * When your @link setIncludePaths() include path@endlink is @c /path/to/my/static/files
     * and you set @c "assets" to the list of @a dirs, requested files like
     * @c/assets/css/style@c.css will always be tried to be served by this plugin. If these files
     * are not found, a 404 status will be returned.
     *
     * If setServeDirsOnly() is set to @c false (the default), the plugin will still try to serve
     * files as static if they end with something that looks like a file extension. Set
     * setServeDirsOnly() to @c true to only serve files as static that start with paths defined
     * here. If you would than request a file like @c/some/where/else/script@c.js it would
     * not be tried to be found in the included directories and the dispatcher would try to
     * find a fitting controller method for it.
     *
     * @sa setServeDirsOnly()
     */
    void setDirs(const QStringList &dirs);

    /**
     * Set this to @c true to only server static files where their path begins with one
     * of the directories set by setDirs(). The default value is @c false.
     *
     * @sa setDirs()
     * @since %Cutelyst 4.0.0
     */
    void setServeDirsOnly(bool dirsOnly);

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
