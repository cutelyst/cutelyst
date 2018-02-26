/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include "protocolfastcgi.h"

#include "socket.h"
#include "wsgi.h"

#include <Cutelyst/Context>

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QTemporaryFile>
#include <QBuffer>

Q_LOGGING_CATEGORY(CWSGI_FCGI, "cwsgi.fcgi")

/*
 * Listening socket file number
 */
#define FCGI_LISTENSOCK_FILENO 0

/*
 * Number of bytes in a FCGI_Header.  Future versions of the protocol
 * will not reduce this number.
 */
#define FCGI_HEADER_LEN  8

/*
 * Value for version component of FCGI_Header
 */
#define FCGI_VERSION_1           1

/*
 * Values for type component of FCGI_Header
 */
#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

/*
 * Value for requestId component of FCGI_Header
 */
#define FCGI_NULL_REQUEST_ID     0

/*
 * Mask for flags component of FCGI_BeginRequestBody
 */
#define FCGI_KEEP_CONN  1

/*
 * Values for role component of FCGI_BeginRequestBody
 */
#define FCGI_RESPONDER  1
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

/*
 * Values for protocolStatus component of FCGI_EndRequestBody
 */
#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3

/*
 * Variable names for FCGI_GET_VALUES / FCGI_GET_VALUES_RESULT records
 */
#define FCGI_MAX_CONNS  "FCGI_MAX_CONNS"
#define FCGI_MAX_REQS   "FCGI_MAX_REQS"
#define FCGI_MPXS_CONNS "FCGI_MPXS_CONNS"

#define WSGI_OK     0
#define WSGI_AGAIN  1
#define WSGI_BODY   2
#define WSGI_ERROR -1

#define FCGI_ALIGNMENT		 8
#define FCGI_ALIGN(n)		 \
    (((n) + (FCGI_ALIGNMENT - 1)) & ~(FCGI_ALIGNMENT - 1))

using namespace CWSGI;

#ifdef Q_CC_MSVC
#pragma pack(push)
#pragma pack(1)
#endif
struct fcgi_record {
    quint8 version;
    quint8 type;
    quint8 req1;
    quint8 req0;
    quint8 cl1;
    quint8 cl0;
    quint8 pad;
    quint8 reserved;
}
#ifdef Q_CC_MSVC
;
#pragma pack(pop)
#else
__attribute__ ((__packed__));
#endif

#ifdef Q_CC_MSVC
#pragma pack(push)
#pragma pack(1)
#endif
struct fcgi_begin_request_body {
    quint16	role;
    quint8		flags;
    quint8		reserved[5];
}
#ifdef Q_CC_MSVC
;
#pragma pack(pop)
#else
__attribute__ ((__packed__));
#endif

ProtocolFastCGI::ProtocolFastCGI(WSGI *wsgi) : Protocol(wsgi)
{
}

ProtocolFastCGI::~ProtocolFastCGI()
{
}

Protocol::Type ProtocolFastCGI::type() const
{
    return FastCGI1;
}

quint32 wsgi_be32(const char *buf) {
    const quint32 *src = reinterpret_cast<const quint32 *>(buf);
    quint32 ret = 0;
    quint8 *ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 24) & 0xff);
    ptr[1] = static_cast<quint8>((*src >> 16) & 0xff);
    ptr[2] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[3] = static_cast<quint8>(*src & 0xff);
    return ret;
}

quint16 wsgi_be16(const char *buf) {
    const quint32 *src = reinterpret_cast<const quint32 *>(buf);
    quint16 ret = 0;
    quint8 *ptr = reinterpret_cast<quint8 *>(&ret);
    ptr[0] = static_cast<quint8>((*src >> 8) & 0xff);
    ptr[1] = static_cast<quint8>(*src & 0xff);
    return ret;
}

quint16 ProtocolFastCGI::addHeader(Socket *wsgi_req, const char *key, quint16 keylen, const char *val, quint16 vallen) const
{
    char *buffer = wsgi_req->buffer + wsgi_req->pktsize;
    char *watermark = wsgi_req->buffer + m_bufferSize;

    if (buffer + keylen + vallen + 2 + 2 >= watermark) {
        qCWarning(CWSGI_FCGI, "unable to add %.*s=%.*s to wsgi packet, consider increasing buffer size", keylen, key, vallen, val);
        return 0;
    }

    if (keylen > 5 && memcmp(key, "HTTP_", 5) == 0) {
        const QString value = QString::fromLatin1(val, vallen);
        if (!wsgi_req->headerHost && memcmp(key + 5, "HOST", 4) == 0) {
            wsgi_req->serverAddress = value;
            wsgi_req->headerHost = true;
            wsgi_req->headers.pushRawHeader(QStringLiteral("HOST"), value);
        } else {
            const QString keyStr = QString::fromLatin1(key + 5, keylen - 5);
            wsgi_req->headers.pushRawHeader(keyStr, value);
        }
    } else if (memcmp(key, "REQUEST_METHOD", 14) == 0) {
        wsgi_req->method = QString::fromLatin1(val, vallen);
    } else if (memcmp(key, "REQUEST_URI", 11) == 0) {
        const char *pch = static_cast<const char *>(memchr(val, '?', vallen));
        if (pch) {
            int pos = pch - val;
            wsgi_req->path = QString::fromLatin1(val + 1, pos - 1);
            wsgi_req->query = QByteArray(pch + 1, vallen - pos - 1);
        } else {
            wsgi_req->path = QString::fromLatin1(val + 1, vallen - 1);
            wsgi_req->query = QByteArray();
        }
    } else if (memcmp(key, "SERVER_PROTOCOL", 15) == 0) {
        wsgi_req->protocol = QString::fromLatin1(val, vallen);
    } else if (memcmp(key, "REMOTE_ADDR", 11) == 0) {
        wsgi_req->remoteAddress.setAddress(QString::fromLatin1(val, vallen));
    } else if (memcmp(key, "REMOTE_PORT", 11) == 0) {
        wsgi_req->remotePort = QByteArray(val, vallen).toUInt();
    } else if (memcmp(key, "CONTENT_TYPE", 12) == 0) {
        if (vallen) {
            wsgi_req->headers.setContentType(QString::fromLatin1(val, vallen));
        }
    } else if (memcmp(key, "CONTENT_LENGTH", 14) == 0) {
        wsgi_req->contentLength = QByteArray(val, vallen).toInt();
    } else if (memcmp(key, "REQUEST_SCHEME", 14) == 0) {
        wsgi_req->isSecure = QByteArray(val, vallen) == "https" ? true : false;
    }

//#ifdef DEBUG
//    qCDebug(CWSGI_FCGI, "add uwsgi var: %.*s = %.*s", keylen, key, vallen, val);
//#endif

    return keylen + vallen + 2 + 2;
}

int ProtocolFastCGI::parseHeaders(Socket *wsgi_req, const char *buf, size_t len) const
{
    size_t j;
    quint8 octet;
    quint32 keylen, vallen;
    for (j = 0; j < len; j++) {
        octet = static_cast<quint8>(buf[j]);
        if (octet > 127) {
            if (j + 4 >= len)
                return -1;
            keylen = wsgi_be32(&buf[j]) ^ 0x80000000;
            j += 4;
        }
        else {
            if (j + 1 >= len)
                return -1;
            keylen = octet;
            j++;
        }
        octet = static_cast<quint8>(buf[j]);
        if (octet > 127) {
            if (j + 4 >= len)
                return -1;
            vallen = wsgi_be32(&buf[j]) ^ 0x80000000;
            j += 4;
        }
        else {
            if (j + 1 >= len)
                return -1;
            vallen = octet;
            j++;
        }

        if (j + (keylen + vallen) > len) {
            return -1;
        }

        if (keylen > 0xffff || vallen > 0xffff)
            return -1;
        quint16 pktsize = addHeader(wsgi_req, buf + j, keylen, buf + j + keylen, vallen);
        if (pktsize == 0)
            return -1;
        wsgi_req->pktsize += pktsize;
        // -1 here as the for() will increment j again
        j += (keylen + vallen) - 1;
    }

    return 0;
}

int ProtocolFastCGI::processPacket(Socket *sock) const
{
    Q_FOREVER {
        if (sock->buf_size >= sizeof(struct fcgi_record)) {
            auto fr = reinterpret_cast<struct fcgi_record *>(sock->buffer);

            quint16 fcgi_len = wsgi_be16(reinterpret_cast<const char *>(&fr->cl1));
            quint32 fcgi_all_len = sizeof(struct fcgi_record) + fcgi_len + fr->pad;
            quint8 fcgi_type = fr->type;
            quint8 *sid = reinterpret_cast<quint8 *>(& sock->stream_id);
            sid[0] = fr->req0;
            sid[1] = fr->req1;

            // if STDIN, end of the loop
            if (fcgi_type == FCGI_STDIN) {
                if (fcgi_len == 0) {
                    memmove(sock->buffer, sock->buffer + fcgi_all_len, sock->buf_size - fcgi_all_len);
                    sock->buf_size -= fcgi_all_len;
                    return WSGI_OK;
                }

                quint16 content_size = sock->buf_size - sizeof(struct fcgi_record);
                if (!writeBody(sock, sock->buffer + sizeof(struct fcgi_record),
                               qMin(content_size, fcgi_len))) {
                    return WSGI_ERROR;
                }

                if (content_size < fcgi_len) {
                    // we still need the rest of the pkt body
                    sock->connState = Socket::ContentBody;
                    sock->pktsize = fcgi_len - content_size;
                    sock->buf_size = fr->pad;
                    return WSGI_BODY;
                }

                memmove(sock->buffer, sock->buffer + fcgi_all_len, sock->buf_size - fcgi_all_len);
                sock->buf_size -= fcgi_all_len;
            } else if (sock->buf_size >= fcgi_all_len) {
                // PARAMS ? (ignore other types)
                if (fcgi_type == FCGI_PARAMS) {
                    if (parseHeaders(sock, sock->buffer + sizeof(struct fcgi_record), fcgi_len)) {
                        return WSGI_ERROR;
                    }
                } else if (fcgi_type == FCGI_BEGIN_REQUEST) {
                    auto brb = reinterpret_cast<struct fcgi_begin_request_body *>(sock->buffer + sizeof(struct fcgi_begin_request_body));
                    sock->headerConnection = (brb->flags & FCGI_KEEP_CONN) ? Socket::HeaderConnectionKeep : Socket::HeaderConnectionClose;
                    sock->contentLength = -1;
                    sock->headers = Cutelyst::Headers();
                    sock->connState = Socket::MethodLine;
                }

                memmove(sock->buffer, sock->buffer + fcgi_all_len, sock->buf_size - fcgi_all_len);
                sock->buf_size -= fcgi_all_len;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return WSGI_AGAIN; // read again
}

bool ProtocolFastCGI::writeBody(Socket *sock, char *buf, qint64 len) const
{
    if (sock->body) {
        return sock->body->write(buf, len) == len;
    }

    if (m_postBuffering && sock->contentLength > m_postBuffering) {
        auto temp = new QTemporaryFile;
        if (!temp->open()) {
            qCWarning(CWSGI_FCGI) << "Failed to open temporary file to store post" << temp->errorString();
            return false;
        }
        sock->body = temp;
    } else if (m_postBuffering && sock->contentLength <= m_postBuffering) {
        auto buffer = new QBuffer;
        buffer->open(QIODevice::ReadWrite);
        buffer->buffer().reserve(sock->contentLength);
        sock->body = buffer;
    } else {
        // Unbuffered
        auto buffer = new QBuffer;
        buffer->open(QIODevice::ReadWrite);
        buffer->buffer().reserve(sock->contentLength);
        sock->body = buffer;
    }

    return sock->body->write(buf, len) == len;
}

int ProtocolFastCGI::wsgi_proto_fastcgi_write(QIODevice *io, Socket *wsgi_req, const char *buf, int len)
{
    // reset for next write
    int write_pos = 0;
    quint32 proto_parser_status = 0;

    Q_FOREVER {
        // fastcgi packets are limited to 64k
        quint8 padding = 0;

        if (proto_parser_status == 0) {
            quint16 fcgi_len = qMin(len - write_pos, 0xffff);
            proto_parser_status = fcgi_len;

            struct fcgi_record fr;
            fr.version = FCGI_VERSION_1;
            fr.type = FCGI_STDOUT;

            quint8 *sid = reinterpret_cast<quint8 *>(&wsgi_req->stream_id);
            fr.req1 = sid[1];
            fr.req0 = sid[0];

            quint16 padded_len = FCGI_ALIGN(fcgi_len);
            if (padded_len > fcgi_len) {
                padding = padded_len - fcgi_len;
            }
            fr.pad = padding;

            fr.reserved = 0;
            fr.cl0 = static_cast<quint8>(fcgi_len & 0xff);
            fr.cl1 = static_cast<quint8>((fcgi_len >> 8) & 0xff);
            if (io->write(reinterpret_cast<const char *>(&fr), sizeof(struct fcgi_record)) != sizeof(struct fcgi_record)) {
                return -1;
            }
        }

        qint64 wlen = io->write(buf + write_pos, proto_parser_status);
        if (padding) {
            io->write("\0\0\0\0\0\0\0\0\0", padding);
        }

        if (wlen > 0) {
            write_pos += wlen;
            proto_parser_status -= wlen;
            if (write_pos == len) {
                return WSGI_OK;
            }
            continue;
        }
        if (wlen < 0) {
            qCWarning(CWSGI_FCGI) << "Writing socket error" << io->errorString();
        }
        return -1;
    }
}

#define FCGI_END_REQUEST_DATA "\1\x06\0\1\0\0\0\0\1\3\0\1\0\x08\0\0\0\0\0\0\0\0\0\0"

void wsgi_proto_fastcgi_endrequest(Socket *wsgi_req, QIODevice *io)
{
    char end_request[] = FCGI_END_REQUEST_DATA;
//    memcpy(end_request, FCGI_END_REQUEST_DATA, 24);
    char *sid = (char *) &wsgi_req->stream_id;
    // update with request id
    end_request[2] = sid[1];
    end_request[3] = sid[0];
    end_request[10] = sid[1];
    end_request[11] = sid[0];
    io->write(end_request, 24);
}

qint64 ProtocolFastCGI::readBody(Socket *sock, QIODevice *io, qint64 bytesAvailable) const
{
    qint64 len;
    QIODevice *body = sock->body;
    quint32 &pad = sock->buf_size;
    while (bytesAvailable && sock->pktsize + pad) {
        // We need to read and ignore ending PAD data
        len = io->read(m_postBuffer, qMin(m_postBufferSize, static_cast<qint64>(sock->pktsize + pad)));
        if (len == -1) {
            sock->connectionClose();
            return -1;
        }
        bytesAvailable -= len;

        if (len > sock->pktsize) {
            // We read past pktsize, so possibly PAD data was read too.
            pad -= len - sock->pktsize;
            len = sock->pktsize;
            sock->pktsize = 0;
        } else {
            sock->pktsize -= len;
        }

        body->write(m_postBuffer, len);
    }

    if ( sock->pktsize + pad == 0) {
        sock->connState = Socket::MethodLine;
    }

    return bytesAvailable;
}

void ProtocolFastCGI::readyRead(Socket *sock, QIODevice *io) const
{
    // Post buffering
    qint64 bytesAvailable = io->bytesAvailable();
    if (sock->connState == Socket::ContentBody) {
        bytesAvailable = readBody(sock, io, bytesAvailable);
        if (bytesAvailable == -1) {
            return;
        }
    }

    do {
        int len = io->read(sock->buffer + sock->buf_size, m_bufferSize - sock->buf_size);
        bytesAvailable -= len;

        if (len > 0) {
            sock->buf_size += len;

            if (!sock->startOfRequest) {
                sock->startOfRequest = sock->engine->time();
            }

            if (sock->buf_size < sizeof(struct fcgi_record)) {
                // not enough data
                continue;
            }

            int ret = processPacket(sock);
            if (ret == WSGI_AGAIN) {
                continue;
            } else if (ret == WSGI_OK) {
                sock->processing = true;
                delete sock->engine->processRequest(sock);
                wsgi_proto_fastcgi_endrequest(sock, io);
                sock->processing = false;

                if (sock->headerConnection == Socket::HeaderConnectionClose) {
                    // Web server did not set FCGI_KEEP_CONN
                    sock->connectionClose();
                    return;
                }

                auto size = sock->buf_size;
                sock->resetSocket();
                sock->buf_size = size;
            } else if (ret == WSGI_BODY) {
                bytesAvailable = readBody(sock, io, bytesAvailable);
                if (bytesAvailable == -1) {
                    return;
                }
            } else {
                // On error disconnect immediately
                io->close();
            }
        } else {
            qCWarning(CWSGI_FCGI) << "Failed to read from socket" << io->errorString();
            break;
        }
    } while (bytesAvailable);
}

bool ProtocolFastCGI::sendHeaders(QIODevice *io, Socket *sock, quint16 status, const QByteArray &dateHeader, const Cutelyst::Headers &headers)
{
    static thread_local QByteArray headerBuffer = ([]() -> QByteArray {
                                                       QByteArray ret;
                                                       ret.reserve(1024);
                                                       return ret;
                                                   }());

    headerBuffer.resize(0);
    headerBuffer.append(QByteArrayLiteral("Status: ") + QByteArray::number(status));

    const auto headersData = headers.data();

    bool hasDate = false;
    auto it = headersData.constBegin();
    const auto endIt = headersData.constEnd();
    while (it != endIt) {
        const QString &key = it.key();
        const QString &value = it.value();
        if (!hasDate && key == QLatin1String("DATE")) {
            hasDate = true;
        }

        QString line(QLatin1String("\r\n") + CWsgiEngine::camelCaseHeader(key) + QLatin1String(": ") + value);
        const QByteArray data = line.toLatin1();
        headerBuffer.append(data);

        ++it;
    }

    if (!hasDate) {
        headerBuffer.append(dateHeader);
    }
    headerBuffer.append("\r\n\r\n", 4);

    return wsgi_proto_fastcgi_write(io, sock, headerBuffer.constData(), headerBuffer.size()) == 0;
}

qint64 ProtocolFastCGI::sendBody(QIODevice *io, Socket *sock, const char *data, qint64 len)
{
    if (wsgi_proto_fastcgi_write(io, sock, data, len) == 0) {
        return len;
    }
    return -1;
}
