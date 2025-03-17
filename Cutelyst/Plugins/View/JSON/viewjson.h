/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWJSON_H
#define VIEWJSON_H

#include <Cutelyst/Plugins/View/json_export.h>
#include <Cutelyst/view.h>

namespace Cutelyst {

class ViewJsonPrivate;
/**
 * \ingroup plugins-view
 * \headerfile "" <Cutelyst/Plugins/View/JSON/viewjson.h>
 * \brief A view that returns stash data in JSON format.
 *
 * The %ViewJson is a view handler that returns Context::stash() data in JSON format. You can
 * limit the exposed stash data by setting the setExposeStash() method to a single key, a list
 * of keys or a regular expression matching specific keys. By default, the complete stash content
 * will be put into the created JSON object. This view also automatically sets the \c Content-Type
 * HTTP response header to \a "application/json".
 */
class CUTELYST_VIEW_JSON_EXPORT ViewJson final : public View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ViewJson)
public:
    /**
     * Constructs a new %ViewJson with the given \a parent and \a name.
     *
     * The \a name can be used to specify different views that can be called either dynamically
     * by Context::setCustomView() or with the \c :View() argument of the RenderView action.
     */
    explicit ViewJson(QObject *parent, const QString &name = {});

    /**  This value defines the format of the JSON byte array produced when rendering the view. */
    enum JsonFormat {
        Indented, /**< The output will be indented */
        Compact   /**< The output will be compact saving space (the default) */
    };
    Q_ENUM(JsonFormat)

    /**
     * Returns the output format of JSON,
     * defaults to ViewJson::Compact.
     */
    [[nodiscard]] JsonFormat outputFormat() const;

    /**
     * Defines the output format of JSON.
     */
    void setOutputFormat(JsonFormat format);

    /**  This value defines which kind of exposition was defined */
    enum ExposeMode {
        All,              /**< The complete stash will be put into the JSON object.
                             (the default) */
        String,           /**< Only the content of a single stash key will be put into
                             the JSON object. */
        StringList,       /**< Only the content of the stash keys in the list of keys
                             will be put into the JSON object. */
        RegularExpression /**< Only the content of the stash keys that match the regular
                             expression will be put into the JSON object. */
    };
    Q_ENUM(ExposeMode)

    /**
     * Returns the expose mode of the stash keys,
     * defaults to everything (ViewJson::All).
     */
    [[nodiscard]] ExposeMode exposeStashMode() const;

    /**
     * Specify which stash \a key is exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::String.
     * Only the content of this single \a key will be exposed to the
     * JSON response.
     */
    void setExposeStash(const QString &key);

    /**
     * Specify which stash \a keys are exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::StringList.
     * Only content of stash keys that are found in the list of \a keys
     * will be exposed to the JSON response.
     */
    void setExposeStash(const QStringList &keys);

    /**
     * Specify which stash keys are exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::RegularExpression.
     * Only keys that match the regular expression \a re will be
     * exposed to the JSON response.
     */
    void setExposeStash(const QRegularExpression &re);

    /**
     * By default this plugin does \b NOT set X-JSON header if the requested client is a
     * Prototype.js with X-JSON support. By setting \a enable to \c true, you can opt-out
     * this behavior so that you do not need to do eval() by your own.
     */
    void setXJsonHeader(bool enable);

    /**
     * Returns true if the X-JSON header should be sent.
     */
    [[nodiscard]] bool xJsonHeader() const;

    QByteArray render(Context *c) const final;
};

} // namespace Cutelyst

#endif // VIEWJSON_H
