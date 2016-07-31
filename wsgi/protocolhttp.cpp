#include "protocolhttp.h"
#include "socket.h"

#include <Cutelyst/Headers>

#include <QVariant>
#include <QIODevice>
#include <QByteArrayMatcher>
#include <QEventLoop>
#include <QTimer>
#include <QDebug>

using namespace CWSGI;

ProtocolHttp::ProtocolHttp(QObject *parent) : Protocol(parent)
{

}

void ProtocolHttp::readyRead()
{
    static QByteArrayMatcher matcher("\r\n");

    auto conn = sender();
    auto sock = qobject_cast<TcpSocket*>(conn);

    int len = sock->read(sock->buf + sock->buf_size, 4096 - sock->buf_size);

    sock->buf_size += len;

    while (sock->last < sock->buf_size) {
        int ix = matcher.indexIn(sock->buf, sock->buf_size, sock->last);
        if (ix != -1) {
            int len = ix - sock->beginLine;
            char *ptr = sock->buf + sock->beginLine;
            sock->beginLine = ix + 2;
            sock->last = sock->beginLine;
            if (sock->connState == 0) {
                processRequest(ptr, len, sock);
                sock->connState = 1;
            } else if (sock->connState == 1) {
                if (len) {
                    processHeader(ptr, len, sock);
                } else {
//                    qDebug() << sock->headers.map();
                    sock->processing = true;
                    sock->engine->processSocket(sock);
                    sock->processing = false;

//                    qDebug() << sock->headers.connection() << QString::compare(sock->headers.connection(), QLatin1String("close"), Qt::CaseInsensitive);
                    if (sock->headerClose == 2) {
                        sock->disconnectFromHost();
                    }

                    sock->headers = Headers();
                    sock->headerClose = 0;
                    sock->connState = 0;
                    sock->beginLine = 0;
                    sock->buf_size = 0;
                    sock->last = 0;
                    sock->start = sock->engine->time();

                    break;
                }
            }

        } else {
            sock->last = sock->buf_size;
        }
    }


}

static inline const char * findSpace(const char *str, int len) {
    const char *space = str;
    while (space < str + len) {
        if (*space == ' ') {
            break;
        }
        ++space;
    }
    return space;
}

static inline const char * findQuestionMarkOrSpace(const char *str, int len) {
    const char *qm = str;
    while (qm < str + len) {
        if (*qm == '?' || *qm == ' ') {
            break;
        }
        ++qm;
    }
    return qm;
}

static inline const char * findNotSpace(const char *str, int len) {
    const char *notSpace = str;
    while (notSpace < str + len) {
        if (*notSpace != ' ') {
            break;
        }
        ++notSpace;
    }
    return notSpace;
}

void ProtocolHttp::processRequest(const char *ptr, int len, Socket *sock)
{
    const char *space = findSpace(ptr, len);

    sock->method = QString::fromLatin1(ptr, space - ptr);

    const char *data = findNotSpace(space, len + (ptr - space));

    space = findQuestionMarkOrSpace(data, len + (ptr - data - 1));
    ++data;// skip slash
    sock->path = QString::fromLatin1(data, space - data);

    if (*space == '?') {
        data = ++space;
        space = findSpace(space, len - (ptr - space));
        sock->query = QByteArray(data, space - data);
    } else {
        sock->query = QByteArray();
    }

    data = findNotSpace(space, len - (ptr - space));
    sock->protocol = QString::fromLatin1(data, len + (ptr - data));
}

static inline const char * findDouble(const char *str, int len) {
    const char *ret = str;
    while (ret < str + len) {
        if (*ret == ':') {
            break;
        }
        ++ret;
    }
    return ret;
}

void ProtocolHttp::processHeader(const char *ptr, int len, Socket *sock)
{
    const char *db = findDouble(ptr, len);
    QString key = QString::fromLatin1(ptr, db - ptr);

    ++db;// skip
    const char *data = findNotSpace(db, len + (ptr - db));
    QString value = QString::fromLatin1(data, len + (ptr - data));

    if (sock->headerClose == 0 && key.compare(QLatin1String("Connection"), Qt::CaseInsensitive) == 0) {
        if (value.compare(QLatin1String("close"), Qt::CaseInsensitive) == 0) {
            sock->headerClose = 2;
        } else {
            sock->headerClose = 1;
        }
    }
    sock->headers.pushHeader(key, value);
}
