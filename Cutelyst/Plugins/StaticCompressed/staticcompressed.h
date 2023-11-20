/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATICCOMPRESSED_H
#define STATICCOMPRESSED_H

#include <Cutelyst/Plugin>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class StaticCompressedPrivate;

/**
 * @ingroup plugins
 * @brief Serve static files compressed on the fly or pre-compressed.
 *
 * The %StaticCompressed plugin for %Cutelyst can be used to serve specific static files like
 * CSS and JavaScript files compressed. It has built in support for
 * <A HREF="https://en.wikipedia.org/wiki/Gzip">gzip</A> and
 * <A HREF="https://en.wikipedia.org/wiki/DEFLATE">DEFLATE</A> compression format and can be
 * extended by external libraries to support the <A
 * HREF="https://en.wikipedia.org/wiki/Brotli">Brotli</A> compression algorithm and to use <A
 * HREF="https://en.wikipedia.org/wiki/Zopfli">Zopfli</A> for @a gzip compression. Beside
 * compressing the raw data on the fly and store the result in a cache directory, it supports
 * pre-compressed files distinguished by file extension in the static source directories. The
 * plugin uses the @a Accept-Encoding HTTP request header to determine the compression methods
 * supported by the user agent. If you do not need this, use the StaticSimple plugin to serve
 * your static files.
 *
 * Beside serving the file content this will also set the respective HTTP header fields
 * @c Content-Type, @c Content-Length, @c Last-Modified, @c Content-Encoding,
 * @c Vary=Accept-Encoding and @c Cache-Control=public.
 *
 * <H3>Compression formats</H3>
 *
 * Support for @a gzip and @a DEFLATE compression format is built in by using the
 * @link QByteArray::qCompress() qCompress()@endlink function. To enable suport for
 * <A HREF="https://en.wikipedia.org/wiki/Brotli">Brotli</A>, build
 * with <TT>-DPLUGIN_STATICCOMPRESSED_BROTLI:BOOL=ON</TT> and have the
 * <A HREF="https://github.com/google/brotli">libbrotlienc</A> development and header files
 * available.
 *
 * To use <A HREF="https://en.wikipedia.org/wiki/Zopfli">Zopfli</A> for the @a gzip compression,
 * build with <TT>-DPLUGIN_STATICCOMPRESSED_ZOPFLI:BOOL=ON</TT> and have the <A
 * HREF="https://github.com/google/zopfli">libzopfli</A> development and header files available.
 * Also set the configuration key @c use_zopfli to @c true. Be aware that @a Zopfli gives better
 * compression rate than default @a gzip but is also much slower. So @a Zopfli is disabled by
 * default even if it is enabled at compilation time.
 *
 * <H3>On the fly compression</H3>
 *
 * Static files of the <A HREF="#configfile">configured</A> @c mime_types or with the configured
 * @c suffixes can be compressed on the fly into a format that is accepted by the requesting user
 * agent. The compressed data is saved into files in the @c cache_diretory specified in the
 * configuration. The cache file name will be the MD5 hash sum of the original local file path
 * together with the file extension indicating the compression format (.br for Brotli, .gz for
 * gzip/Zopfli and .deflate for DEFLATE). If the modification time of the original file is newer
 * than the modification time of the cached compressed file, the file will be compressed again.
 * It is safe to clean the content of the cache directory - the files will then be recompressed
 * on the next request. On the fly compression can be disabled by setting @c on_the_fly_compression
 * to @c false in the configuration file.
 *
 * <H3>Pre-compressed files</H3>
 *
 * Beside the cached on the fly compression it is also possible to serve pre-compressed static
 * files that are available from the same directories as the original files. The %StaticCompressed
 * plugin will try to find a compressed file at the same path as the original file appended by an
 * extension indicating the compression method. So if you have for example boostrap.min.css and
 * bootstrap.min.css.gz in your static files directory, the plugin will serve the compressed
 * variant if the requesting user agent supports the gzip encoding. The delivery of pre-compressed
 * files can be disabled by setting @c check_pre_compressed to @c false in the configuration file.
 *
 * <H3>Used file extensions/suffixes</H3>
 *
 * @li .br - Brotli compressed files
 * @li .gz - gzip/Zopfli compressed files
 * @li .deflate - DEFLATE compressed files
 *
 * <H3>Only serve specific request paths</H3>
 *
 * You can use setDirs() to set a list of directories/paths below your web root where files should
 * always be served by this plugin. By default, the plugin also tries to serve files from other
 * paths when they have a file extension when they not start with one of these paths. You can
 * set setServeDirsOnly() to @c true (since %Cutelyst 4.0.0) to only serve files beginning with
 * these paths. Have a look at setDirs() to learn more.
 *
 * <H3 ID="configfile">Runtime configuration</H3>
 *
 * The plugin offers some configuration options that can be set in the %Cutelyst application
 * configuration file in the @c Cutelyst_StaticCompressed_Plugin section. You can override the
 * defaults by setting a QVariantMap with selected default values to the constructor.
 *
 * @configblock{cache_directory,string,QStandardPaths::CacheLocation + /compressed-static}
 * Sets the directory path where on the fly compressed data is saved.
 * @endconfigblock
 *
 * @configblock{mime_types,string,text/css\,application/javascript\,text/javascript}
 * Comma separated list of MIME types that should be compressed.
 * @endconfigblock
 *
 * @configblock{suffixes,string,js.map\,css.map\,min.js.map\,min.css.map}
 * Comma separted list of file suffixes/extensions that should be compressed.
 * @endconfigblock
 *
 * @configblock{check_pre_compressed,bool,true}
 * Enables or disables the check for pre compressed files.
 * @endconfigblock
 *
 * @configblock{on_the_fly_compression,bool,true}
 * Enables or disables the compression on the fly.
 * @endconfigblock
 *
 * @configblock{zlib_compression_level,integer,9}
 * Compression level for built in zlib based compression between 0 and 9, with 9 corresponding
 * to the best compression.
 * @endconfigblock
 *
 * @configblock{brotli_quality_level,integer,11}
 * Quality level for @a Brotli compression between 0 and 11, with 11 corresponding to the
 * best compression.
 * @endconfigblock
 *
 * @configblock{use_zopfli,bool,false}
 * Enables the optional use of @a Zopfli for the @a gzip compression. Note that @a Zopfli gives
 * much better compression results for the cost of a slower compression.
 * @endconfigblock
 *
 * @configblock{zopfli_iterations,integer,15}
 * Number of iterations used for @a Zopfli compression, more gives better compression but is slower.
 * @endconfigblock
 *
 * <H3>Usage example</H3>
 *
 * @code{.cpp}
 * #include <Cutelyst/Plugins/StaticCompressed/StaticCompressed>
 *
 * bool MyCutelystApp::init()
 * {
 *     // other initialization stuff
 *     // ...
 *
 *     // constructs a new plugin and sets the default value for
 *     // the config file option use_zopfli to true
 *     auto staticCompressed = new StaticCompressed(this, {{"use_zopfli", true}});
 *     staticCompressed->setIncludePaths({QStringLiteral("/path/to/my/static/files")});
 *
 *     // maybe more initialization stuff
 *     //...
 * }
 * @endcode
 *
 * <H3>Build configuration</H3>
 *
 * @li @c -DPLUGIN_STATICCOMPRESSED@c:BOOL=ON - enables the build of the plugin (default: @c off)
 * @li @c -DPLUGIN_STATICCOMPRESSED_ZOPFLI@c:BOOL=ON - enables the @a Zopfli support for gzip,
 * <A HREF="https://github.com/google/zopfli">libzopfli</A> development and header files have to be
 * present (default: @c off)
 * @li @c -DPLUGIN_STATICCOMPRESSED_BROTLI@c:BOOL=ON - enables the @a Brotli support,
 * <A HREF="https://github.com/google/brotli">libbrotlienc</A> development and header files have to
 * be present (default: @c off)
 *
 * Since %Cutelyst 2.0.0 you can check if \c CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI and/or
 * \c CUTELYST_STATICCOMPRESSED_WITH_BROTLI are defined if you need to know that the plugin supports
 * that compressions.
 *
 * @par Logging category
 * @c cutelyst.plugin.csrfprotection
 *
 * @since %Cutelyst 1.11.0
 * @sa StaticSimple
 * @headerfile "" <Cutelyst/Plugins/StaticCompressed/StaticCompressed>
 */
class CUTELYST_PLUGIN_STATICCOMPRESSED_EXPORT // clazy:exclude=ctor-missing-parent-argument
    StaticCompressed : public Plugin
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(StaticCompressed) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(StaticCompressed)
public:
    /**
     * Constructs a new %StaticCompressed plugin with the given @a parent.
     */
    explicit StaticCompressed(Application *parent);

    /**
     * Constructs a new %StaticCompressed plugin with the given @a parent and @a defaultConfig.
     *
     * Use the @a defaultConfig to set default values for the configuration entries from
     * the <A HREF="#configfile">configuration file</A>.
     *
     * @since %Cutelyst 4.0.0
     */
    StaticCompressed(Application *parent, const QVariantMap &defaultConfig);

    /**
     * Deconstructs the StaticCompressed object.
     */
    ~StaticCompressed() override;

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
     * Configures the plugin by reading the @c Cutelyst_StaticCompressed_Plugin section
     * from the %Cutelyst application configuration file and connects to the
     * Application::beforePrepareAction() signal. Returns @c false if the cache directory
     * can not be created or if it not exists.
     */
    bool setup(Application *app) override;

private:
    std::unique_ptr<StaticCompressedPrivate> d_ptr;
};

} // namespace Cutelyst

#endif // STATICCOMPRESSED_H
