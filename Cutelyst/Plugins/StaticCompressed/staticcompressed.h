/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef STATICCOMPRESSED_H
#define STATICCOMPRESSED_H

#include <Cutelyst/Plugin>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class StaticCompressedPrivate;

/**
 * @brief Deliver static files compressed on the fly or precompressed.
 *
 * The StaticCompressed plugin for Cutelyst can be used to deliver specific static files like
 * CSS and JavaScript files compressed. It has built in support for
 * <A HREF="https://en.wikipedia.org/wiki/Gzip">gzip</A> and
 * <A HREF="https://en.wikipedia.org/wiki/DEFLATE">DEFLATE</A> compression format and can be extended
 * by external libraries to support the <A HREF="https://en.wikipedia.org/wiki/Brotli">Brotli</A>
 * compression algorithm and to use <A HREF="https://en.wikipedia.org/wiki/Zopfli">Zopfli</A> for @a gzip
 * compression. Beside compressing the raw data on the fly and store the result in a cache directory,
 * it supports pre-compressed files distinguished by file extension in the static source directories.
 * The plugin uses the @a Accept-Encoding HTTP request header to determine the compression methods
 * supported by the user agent. If you do not need this, use the StaticSimple plugin to deliver your
 * static files.
 *
 * <H3>Compression formats</H3>
 *
 * Support for @a gzip and @a DEFLATE compression format is built in by using the qCompress() function.
 * To enable suport for <A HREF="https://en.wikipedia.org/wiki/Brotli">Brotli</A>, build with
 * @c-DPLUGIN_STATICCOMPRESSED_BROTLI@c:BOOL=ON and have the <A HREF="https://github.com/google/brotli">libbrotlienc</A>
 * development and header files available. To use <A HREF="https://en.wikipedia.org/wiki/Zopfli">Zopfli</A>
 * for the @a gzip compression, build with @c-DPLUGIN_STATICCOMPRESSED_ZOPFLI@c:BOOL=ON and have the
 * <A HREF="https://github.com/google/zopfli">libzopfli</A> development and header files available. Also
 * set the configuration key @c use_zopfli to @c true. Be aware that @a Zopfli gives better compression
 * rate than default @a gzip but is also much slower. So @a Zopfli is disabled by default even if it is
 * enabled at compilation time.
 *
 * <H3>On the fly compression</H3>
 *
 * Static files of the configured @c mime_types or with the configured @c suffixes can be compressed on the
 * fly into a format that is accepted by the requesting user agent. The compressed data is saved into files
 * in the @c cache_diretory specified in the configuration. The cache file name will be the MD5 hash sum
 * of the original local file path together with the file extension indicating the compression format
 * (.br for Brotli, .gz for gzip/Zopfli and .deflate for DEFLATE). If the modification time of the original
 * file is newer than the modification time of the cached compressed file, the file will be compressed again.
 * It is safe to clean the content of the cache directory - the files will than be recompressed on the next
 * request. On the fly compression can be disabled by setting @c on_the_fly_compression to @c false in the
 * configuration file.
 *
 * <H3>Pre-compressed files</H3>
 *
 * Beside the cached on the fly compression it is also possible to deliver pre-comrpessed static files that
 * are saved in the same place as the original files are. The StaticCompressed plugin will try to find a
 * compressed file at the same path as the original file appended by an extension indicating the compression
 * method. So if you have for example boostrap.min.css and bootstrap.min.css.gz in your static files directory,
 * the plugin will deliver the compressed variant if the requesting user agent supports the gzip encoding. The
 * delivery of pre-compressed files can be disabled by setting @c check_pre_compressed to @c false in the
 * configuration file.
 *
 * <H3>Used file extensions/suffixes</H3>
 *
 * @li .br - Brotli compressed files
 * @li .gz - gzip/Zopfli compressed files
 * @li .deflate - DEFLATE compressed files
 *
 * <H3>Runtime configuration</H3>
 *
 * The plugin offers some configuration options that can be set in the Cutelyst application configuration
 * file in the @c Cutelyst_StaticCompressed_Plugin section.
 * @li @c cache_directory - string value, sets the directory where on the fly compressed data is saved (default:
 * QStandardPaths::CacheLocation + /compressed-static)
 * @li @c mime_types - string value, comma separated list of MIME types that should be compressed (default:
 * text/css,application/javascript)
 * @li @c suffixes - string value, comma separted list of file suffixes/extensions that should be compressed
 * (default: js.map,css.map,min.js.map,min.css.map)
 * @li @c check_pre_compressed - boolean value, enables or disables the check for pre compressed files (default: true)
 * @li @c on_the_fly_compression - boolean value, enables or disables the compression on the fly (default: true)
 * @li @c zlib_compression_level - integer value, compression level for built in zlib based compression between
 * 0 and 9, with 9 corresponding to the greatest compression (default: 9)
 * @li @c brotli_quality_level - integer value, quality level for optional @a Brotli compression between 0 and 11,
 * with 11 corresponding to the greates compression (default: 11)
 * @li @c use_zopfli - boolean value, enables the optional use of @a Zopfli for the @a gzip compression if available
 * @li @c zopfli_iterations - integer value, number of iterations used for @a Zopfli compression, more gives more
 * compression but is slower (default: 15)
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
 *     auto staticCompressed = new StaticCompressed(this);
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
 * Since Cutelyst 2.0.0 you can check if \c CUTELYST_STATICCOMPRESSED_WITH_ZOPFLI and/or
 * \c CUTELYST_STATICCOMPRESSED_WITH_BROTLI are defined if you need to know that the plugin supports
 * that compressions.
 *
 * @since Cutelyst 1.11.0
 * @headerfile "" <Cutelyst/Plugins/StaticCompressed/StaticCompressed>
 */
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
    virtual ~StaticCompressed() override;

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
     * Application::beforePrepareAction() signal. Returns @c false if the cache directory
     * can not be created if it not exists.
     */
    virtual bool setup(Application *app) override;

protected:
    QScopedPointer<StaticCompressedPrivate> d_ptr;
};

} // namespace Cutelyst

#endif // STATICCOMPRESSED_H
