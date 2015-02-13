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

#ifndef MULTIPARTFORMDATAINTERNAL_H
#define MULTIPARTFORMDATAINTERNAL_H

#include <Cutelyst/Upload>

namespace Cutelyst {

class MultiPartFormDataParserPrivate;
class MultiPartFormDataParser
{
    Q_DECLARE_PRIVATE(MultiPartFormDataParser)
public:
    MultiPartFormDataParser();
    virtual ~MultiPartFormDataParser();

    /**
     * @brief MultiPartFormDataInternal
     * @param contentType can be the whole HTTP Content-Type
     * header or just it's value
     * @param body
     */
    virtual Uploads parse(QIODevice *body, const QString &contentType, int bufferSize = 4096);

protected:
    MultiPartFormDataParserPrivate *d_ptr;
};

}

#endif // MULTIPARTFORMDATAINTERNAL_H
