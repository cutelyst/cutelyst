#ifndef MULTIPARTFORMDATAINTERNAL_H
#define MULTIPARTFORMDATAINTERNAL_H

#include "upload.h"

namespace Cutelyst {

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

class MultiPartFormDataInternal
{
public:
    /**
    * @brief MultiPartFormDataInternal
    * @param contentType can be the whole HTTP Content-Type
    * header or just it's value
    * @param body
    */
    MultiPartFormDataInternal(const QByteArray &contentType, QIODevice *body);

    Uploads parse();

private:
    bool execute(char *buffer);
    int findBoundary(char *buffer, int len, ParserState &state, int &boundaryPos);

    QByteArray m_boundary;
    int m_boundaryLength = 0;
    QIODevice *m_body;
};

}

#endif // MULTIPARTFORMDATAINTERNAL_H
