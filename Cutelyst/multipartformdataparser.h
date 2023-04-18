/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MULTIPARTFORMDATAINTERNAL_H
#define MULTIPARTFORMDATAINTERNAL_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/upload.h>

namespace Cutelyst {

class CUTELYST_LIBRARY MultiPartFormDataParser
{
public:
    /**
     * @brief Parser for multipart/formdata
     * @param body
     * @param contentType can be the whole HTTP Content-Type header or just it's value
     * @param bufferSize is the internal buffer size used to parse
     */
    static Uploads parse(QIODevice *body, QStringView contentType, int bufferSize = 4096);
};

} // namespace Cutelyst

#endif // MULTIPARTFORMDATAINTERNAL_H
