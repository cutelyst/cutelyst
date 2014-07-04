#include "multipartformdatainternal.h"
#include "common.h"

#include <QRegularExpression>

using namespace Cutelyst;

MultiPartFormDataInternal::MultiPartFormDataInternal(const QByteArray &contentType, QIODevice *body) :
    m_body(body)
{
    QRegularExpression re("boundary=([^\";]+)");
    QRegularExpressionMatch match = re.match(contentType);
    if (match.hasMatch()) {
        m_boundary = "--" + match.captured(1).toLocal8Bit();
        m_boundaryLength = m_boundary.size();
    }
    qDebug() << "Boudary is" << m_boundary << m_boundaryLength;
}

Uploads MultiPartFormDataInternal::parse()
{
    if (m_boundary.isEmpty()) {
        return Uploads();
    }

    Uploads ret;
    m_body->seek(0);

    char *buffer = new char[4096];
    execute(buffer);
    delete [] buffer;
    m_body->seek(0);


    return ret;
}

bool MultiPartFormDataInternal::execute(char *buffer)
{
    qDebug() << "execute";
//    UploadPrivate *priv = 0;
    QByteArray header;
    int boundaryPos = 0;
    ParserState state = FindBoundary;

    while (!m_body->atEnd()) {
        int i = 0;

        qint64 len = m_body->read(buffer, 4096);
        qDebug() << "execute" << len;

        while (i < len) {

            switch (state) {
            case FindBoundary:
                i += findBoundary(buffer + i, len, state, boundaryPos);
                break;
            case EndBoundaryCR:
                // TODO the "--" case
                if (buffer[i] != '\r') {
                    return false;
                }
                state = EndBoundaryLF;
                break;
            case EndBoundaryLF:
                if (buffer[i] != '\n') {
                    return false;
                }
                header.clear();
                state = StartHeaders;
                break;
            case StartHeaders:
                qCDebug(CUTELYST_MULTIPART) << "StartHeaders" << m_body->pos() - len + i;
                if (buffer[i] == '\r') {
                    state = EndHeaders;
                } else if (buffer[i] == '-') {
                    qCDebug(CUTELYST_MULTIPART) << "StartHeaders done";
                    return false;
                } else {
                    char *pch = strchr(buffer + i, '\r');
                    if (pch == NULL) {
                        header += QByteArray::fromRawData(buffer + i, len - i);
                        i = len;
                    } else {
                        header += QByteArray::fromRawData(buffer + i, pch - buffer - i);
                        i = pch - buffer;
                        state = FinishHeader;
                    }
                }
                break;
            case FinishHeader:
                qCDebug(CUTELYST_MULTIPART) << "FinishHeader" << header;
                header.clear();
                if (buffer[i] == '\n') {
                    state = StartHeaders;
                } else {
                    return false;
                }
                break;
            case EndHeaders:
                qCDebug(CUTELYST_MULTIPART) << "EndHeaders";
                if (buffer[i] == '\n') {
                    state = StartData;
                } else {
                    return false;
                }
                break;
            case StartData:
                qCDebug(CUTELYST_MULTIPART) << "StartData" << m_body->pos() - len + i;
                if (buffer[i] == '\r') {
                    qCDebug(CUTELYST_MULTIPART) << "StartData NULL FILE?";
                }
                state = FindBoundary;
            default:
                break;
            }
            ++i;
        }
    }
    return true;
}

int MultiPartFormDataInternal::findBoundary(char *buffer, int len, ParserState &state, int &boundaryPos)
{
    int i = 0;
    while (i < len) {
        if (buffer[i] == m_boundary[boundaryPos]) {
            if (++boundaryPos == m_boundaryLength) {
                qCDebug(CUTELYST_MULTIPART, "FindBoundary: %llu", m_body->pos() - len + i);
                boundaryPos = 0;
                state = EndBoundaryCR;
                return i;
            }
        } else {
            boundaryPos = 0;
        }
        ++i;
    }
    return i;
}
