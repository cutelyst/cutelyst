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
    execute();

    return ret;
}

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

bool MultiPartFormDataInternal::execute()
{
    qDebug() << "execute";
//    UploadPrivate *priv = 0;
    QByteArray header;
    int boundaryPos = 0;
    ParserState state = FindBoundary;

    while (!m_body->atEnd()) {
        int i = 0;
        const QByteArray &buffer = m_body->read(4096);

        qint64 len = buffer.size();
        qDebug() << "execute" << len;
        while (i < len) {

            switch (state) {
            case FindBoundary:
                if (buffer.at(i) == m_boundary[boundaryPos]) {
                    qDebug() << "execute" << buffer.at(i) << boundaryPos << m_boundaryLength;
                    if (++boundaryPos == m_boundaryLength) {
                        qCDebug(CUTELYST_MULTIPART, "FindBoundary: %llu", m_body->pos() - len + i);
                        boundaryPos = 0;
                        state = EndBoundaryCR;
                    }
                } else {
                    boundaryPos = 0;
                }
                break;
            case EndBoundaryCR:
                // TODO the "--" case
                if (buffer.at(i) != '\r') {
                    return false;
                }
                state = EndBoundaryLF;
                break;
            case EndBoundaryLF:
                if (buffer.at(i) != '\n') {
                    return false;
                }
                header.clear();
                state = StartHeaders;
                break;
            case StartHeaders:
                qCDebug(CUTELYST_MULTIPART) << "StartHeaders" << m_body->pos() - len + i;
                if (buffer.at(i) == '\r') {
                    state = EndHeaders;
                } else if (buffer.at(i) == '-') {
                    qCDebug(CUTELYST_MULTIPART) << "StartHeaders done";
                    return false;
                } else {
                    int cr = buffer.indexOf('\r', i);
                    qCDebug(CUTELYST_MULTIPART) << "StartHeaders cr" << cr;

                    if (cr == -1) {
                        header += buffer.mid(i, len - i);
                        i = len;
                    } else {
                        header += buffer.mid(i, cr - i);
                        qCDebug(CUTELYST_MULTIPART) << "StartHeaders mid" << buffer.mid(i, cr - i);
                        i = cr;
                        state = FinishHeader;
                    }
                }
                break;
            case FinishHeader:
                qCDebug(CUTELYST_MULTIPART) << "FinishHeader" << header;
                header.clear();
                if (buffer.at(i) == '\n') {
                    state = StartHeaders;
                } else {
                    return false;
                }
                break;
            case EndHeaders:
                qCDebug(CUTELYST_MULTIPART) << "EndHeaders";
                if (buffer.at(i) == '\n') {
                    state = StartData;
                } else {
                    return false;
                }
                break;
            case StartData:
                qCDebug(CUTELYST_MULTIPART) << "StartData" << m_body->pos() - len + i;
                if (buffer.at(i) == '\r') {
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
