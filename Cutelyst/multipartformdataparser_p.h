/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef MULTIPARTFORMDATA_P_H
#define MULTIPARTFORMDATA_P_H

#include "multipartformdataparser.h"

#include <QByteArrayMatcher>

namespace Cutelyst {

class MultiPartFormDataParserPrivate
{
    Q_GADGET
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
    Q_ENUM(ParserState)

    static Uploads execute(char *buffer, int bufferSize, QIODevice *body, const QByteArray &boundary);
    static inline int findBoundary(char *buffer, int len, const QByteArrayMatcher &matcher, int boundarySize, ParserState &state);
};

} // namespace Cutelyst

#endif // MULTIPARTFORMDATA_P_H
