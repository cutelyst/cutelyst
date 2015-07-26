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

#ifndef MULTIPARTFORMDATA_P_H
#define MULTIPARTFORMDATA_P_H

#include "multipartformdataparser.h"

namespace Cutelyst {

class MultiPartFormDataParserPrivate
{
public:
    enum ParserState {
        FindBoundary,
        EndBoundaryCR,
        EndBoundaryLF,
        StartHeaders,
        FinishHeader,
        EndHeaders,
        StartData,
        EndData
    };

    Uploads execute(char *buffer, int bufferSize);
    inline int findBoundary(char *buffer, int len, ParserState &state, int &boundaryPos);

    QIODevice *body;
    char *boundary;
    int boundaryLength;

};

}

#endif // MULTIPARTFORMDATA_P_H
