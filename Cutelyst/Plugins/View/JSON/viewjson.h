/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWJSON_H
#define VIEWJSON_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/view.h>

namespace Cutelyst {

class ViewJsonPrivate;
class CUTELYST_VIEW_JSON_EXPORT ViewJson final : public View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ViewJson)
public:
    /**
     * Main constructor
     */
    explicit ViewJson(QObject *parent, const QString &name = QString());

    /**  This value defines the format of the JSON byte array produced when rendering the view */
    enum JsonFormat {
        Indented, /**< The output will be indented */
        Compact   /**< The output will be compact saving space */
    };
    Q_ENUM(JsonFormat)

    /**
     * Returns the output format of JSON,
     * defaults to Compact
     */
    JsonFormat outputFormat() const;

    /**
     * Defines the output format of JSON
     */
    void setOutputFormat(JsonFormat format);

    /**  This value defines which kind of exposition was defined */
    enum ExposeMode {
        All,
        String,
        StringList,
        RegularExpression
    };
    Q_ENUM(ExposeMode)

    /**
     * Returns the expose mode of the stash keys,
     * defaults to everything (All)
     */
    ExposeMode exposeStashMode() const;

    /**
     * Specify which stash key is exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::String
     */
    void setExposeStash(const QString &key);

    /**
     * Specify which stash keys are exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::StringList
     */
    void setExposeStash(const QStringList &keys);

    /**
     * Specify which stash keys are exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::RegularExpression
     */
    void setExposeStash(const QRegularExpression &re);

    /**
     * By default this plugin does \b NOT sets X-JSON header if the requested client is a Prototype.js with X-JSON support.
     * By setting true, you can opt-out this behavior so that you do not need to do eval() by your own.
     */
    void setXJsonHeader(bool enable);

    /**
     * Returns true if the X-JSON header should be sent
     */
    bool xJsonHeader() const;

    QByteArray render(Context *c) const final;
};

} // namespace Cutelyst

#endif // VIEWJSON_H
