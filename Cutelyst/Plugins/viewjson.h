/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef VIEWJSON_H
#define VIEWJSON_H

#include <Cutelyst/view.h>

namespace Cutelyst {

class ViewJsonPrivate;
/**
 * ViewJSON class is a Cutelyst View handler that returns stash
 * data in JSON format.
 */
class ViewJson : public Cutelyst::View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ViewJson)
public:
    explicit ViewJson(Application *app = 0);
    virtual ~ViewJson();

    enum JsonFormat {
        Indented,
        Compact
    };

    /**
     * Returns the output format of JSON,
     * defaults to Compact
     */
    JsonFormat outputFormat() const;

    /**
     * Defines the output format of JSON
     */
    void setOutputFormat(JsonFormat format);

    enum ExposeMode {
        All,
        String,
        StringList,
        RegularExpression
    };
    /**
     * Returns the expose mode of the stash keys,
     * defaults to everything (All)
     */
    ExposeMode exposeStashMode() const;

    /**
     * Specify which stash key is exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::String
     */
    void setExposeStashString(const QString &key);

    /**
     * Specify which stash keys are exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::StringList
     */
    void setExposeStashStringList(const QStringList &keys);

    /**
     * Specify which stash keys are exposed as a JSON response,
     * this will change exposeStashMode() to ViewJson::RegularExpression
     */
    void setExposeStashRegularExpression(const QRegularExpression &re);

    virtual bool render(Cutelyst::Context *c) const Q_DECL_FINAL;

protected:
    ViewJsonPrivate *d_ptr;
};

}

#endif // VIEWJSON_H
