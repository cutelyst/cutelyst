/*
 * SPDX-FileCopyrightText: (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "protocolfastcgi.h"

#include "server.h"
#include "socket.h"

#include <Cutelyst/Context>

#include <QBuffer>
#include <QCoreApplication>
#include <QLoggingCategory>
#include <QTemporaryFile>

Q_LOGGING_CATEGORY(C_SERVER_FCGI, "cutelyst.server.fcgi", QtWarningMsg)

namespace {
/*
 * Value for version component of FCGI_Header
 */
constexpr auto FCGI_VERSION_1 = 1;

/*
 * Values for type component of FCGI_Header
 */
constexpr auto FCGI_BEGIN_REQUEST = 1;
constexpr auto FCGI_PARAMS        = 4;
constexpr auto FCGI_STDIN         = 5;
constexpr auto FCGI_STDOUT        = 6;

/*
 * Mask for flags component of FCGI_BeginRequestBody
 */
constexpr auto FCGI_KEEP_CONN = 1;

constexpr auto WSGI_OK    = 0;
constexpr auto WSGI_AGAIN = 1;
constexpr auto WSGI_BODY  = 2;
constexpr auto WSGI_ERROR = -1;

#define FCGI_ALIGNMENT 8
#define FCGI_ALIGN(n) (((n) + (FCGI_ALIGNMENT - 1)) & ~(FCGI_ALIGNMENT - 1))

struct fcgi_record {
    quint8 version;
    quint8 type;
    quint8 req1;
    quint8 req0;
    quint8 cl1;
    quint8 cl0;
    quint8 pad;
    quint8 reserved;
};

#ifdef Q_CC_MSVC
#    pragma pack(push)
#    pragma pack(1)
#endif
struct fcgi_begin_request_body {
    quint16 role;
    quint8 flags;
    quint8 reserved[5];
}
#ifdef Q_CC_MSVC
;
#    pragma pack(pop)
#else
__attribute__((__packed__));
#endif
} // namespace

using namespace Cutelyst;

ProtocolFastCGI::ProtocolFastCGI(Server *server)
    : Protocol(server)
{
}

ProtocolFastCGI::~ProtocolFastCGI()
{
}

Protocol::Type ProtocolFastCGI::type() const
{
    return Protocol::Type::FastCGI1;
}

quint16 ProtocolFastCGI::addHeader(ProtoRequestFastCGI *request,
                                   const char *key,
                                   quint16 keylen,
                                   const char *val,
                                   quint16 vallen) const
{
    char *buffer          = request->buffer + request->pktsize;
    const char *watermark = request->buffer + m_bufferSize;

    if (buffer + keylen + vallen + 2 + 2 >= watermark) {
        qCWarning(C_SERVER_FCGI,
                  "unable to add %.*s=%.*s to wsgi packet, consider increasing buffer size",
                  keylen,
                  key,
                  vallen,
                  val);
        return 0;
    }

    if (keylen > 5 && memcmp(key, "HTTP_", 5) == 0) {
        const auto value = QByteArray(val, vallen);
        if (!request->headerHost && memcmp(key + 5, "HOST", 4) == 0) {
            request->serverAddress = value;
            request->headerHost    = true;
            request->headers.pushHeader(QByteArrayLiteral("Host"), value);
        } else {
            const auto keyStr = QByteArray(key + 5, keylen - 5).replace('_', '-');
            request->headers.pushHeader(keyStr, value);
        }
    } else if (memcmp(key, "REQUEST_METHOD", 14) == 0) {
        request->method = QByteArray(val, vallen);
    } else if (memcmp(key, "REQUEST_URI", 11) == 0) {
        const char *pch = static_cast<const char *>(memchr(val, '?', vallen));
        if (pch) {
            int pos = int(pch - val);
            request->setPath(const_cast<char *>(val), pos);
            request->query = QByteArray(pch + 1, vallen - pos - 1);
        } else {
            request->setPath(const_cast<char *>(val), vallen);
            request->query = QByteArray();
        }
    } else if (memcmp(key, "SERVER_PROTOCOL", 15) == 0) {
        request->protocol = QByteArray(val, vallen);
    } else if (memcmp(key, "REMOTE_ADDR", 11) == 0) {
        request->remoteAddress.setAddress(QString::fromLatin1(val, vallen));
    } else if (memcmp(key, "REMOTE_PORT", 11) == 0) {
        request->remotePort = quint16(QByteArray(val, vallen).toUInt());
    } else if (memcmp(key, "CONTENT_TYPE", 12) == 0) {
        if (vallen) {
            request->headers.setContentType(QByteArray{val, vallen});
        }
    } else if (memcmp(key, "CONTENT_LENGTH", 14) == 0) {
        request->contentLength = QByteArray(val, vallen).toInt();
    } else if (memcmp(key, "REQUEST_SCHEME", 14) == 0) {
        request->isSecure = QByteArray(val, vallen) == "https" ? true : false;
    }

    // #ifdef DEBUG
    //     qCDebug(C_SERVER_FCGI, "add server var: %.*s = %.*s", keylen, key, vallen, val);
    // #endif

    return keylen + vallen + 2 + 2;
}

int ProtocolFastCGI::parseHeaders(ProtoRequestFastCGI *request, const char *buf, quint16 len) const
{
    quint32 j = 0;
    while (j < len) {
        quint32 keylen;
        auto octet = static_cast<quint8>(buf[j]);
        if (octet > 127) {
            if (j + 4 >= len) {
                return -1;
            }

            // Ignore first bit
            keylen = net_be32(&buf[j]) ^ 0x80000000;
            j += 4;
        } else {
            if (++j >= len) {
                return -1;
            }
            keylen = octet;
        }

        quint32 vallen;
        octet = static_cast<quint8>(buf[j]);
        if (octet > 127) {
            if (j + 4 >= len) {
                return -1;
            }

            // Ignore first bit
            vallen = net_be32(&buf[j]) ^ 0x80000000;
            j += 4;
        } else {
            if (++j >= len) {
                return -1;
            }
            vallen = octet;
        }

        if (j + (keylen + vallen) > len || keylen > 0xffff || vallen > 0xffff) {
            return -1;
        }

        quint16 pktsize =
            addHeader(request, buf + j, quint16(keylen), buf + j + keylen, quint16(vallen));
        if (pktsize == 0) {
            return -1;
        }
        request->pktsize += pktsize;

        j += keylen + vallen;
    }

    return 0;
}

int ProtocolFastCGI::processPacket(ProtoRequestFastCGI *request) const
{
    Q_FOREVER
    {
        if (request->buf_size >= int(sizeof(struct fcgi_record))) {
            const auto *fr = reinterpret_cast<struct fcgi_record *>(request->buffer);

            quint8 fcgi_type    = fr->type;
            auto fcgi_len       = quint16(fr->cl0 | (fr->cl1 << 8));
            qint32 fcgi_all_len = sizeof(struct fcgi_record) + fcgi_len + fr->pad;
            request->stream_id  = quint16(fr->req0 | (fr->req1 << 8));

            // if STDIN, end of the loop
            if (fcgi_type == FCGI_STDIN) {
                if (fcgi_len == 0) {
                    memmove(request->buffer,
                            request->buffer + fcgi_all_len,
                            size_t(request->buf_size - fcgi_all_len));
                    request->buf_size -= fcgi_all_len;
                    return WSGI_OK;
                }

                int content_size = request->buf_size - int(sizeof(struct fcgi_record));
                if (!writeBody(request,
                               request->buffer + sizeof(struct fcgi_record),
                               qMin(content_size, int(fcgi_len)))) {
                    return WSGI_ERROR;
                }

                if (content_size < fcgi_len) {
                    // we still need the rest of the pkt body
                    request->connState = ProtoRequestFastCGI::ContentBody;
                    request->pktsize   = quint16(fcgi_len - content_size);
                    request->buf_size  = fr->pad;
                    return WSGI_BODY;
                }

                memmove(request->buffer,
                        request->buffer + fcgi_all_len,
                        size_t(request->buf_size - fcgi_all_len));
                request->buf_size -= fcgi_all_len;
            } else if (request->buf_size >= fcgi_all_len) {
                // PARAMS ? (ignore other types)
                if (fcgi_type == FCGI_PARAMS) {
                    if (parseHeaders(
                            request, request->buffer + sizeof(struct fcgi_record), fcgi_len)) {
                        return WSGI_ERROR;
                    }
                } else if (fcgi_type == FCGI_BEGIN_REQUEST) {
                    const auto *brb = reinterpret_cast<struct fcgi_begin_request_body *>(
                        request->buffer + sizeof(struct fcgi_begin_request_body));
                    request->headerConnection = (brb->flags & FCGI_KEEP_CONN)
                                                    ? ProtoRequestFastCGI::HeaderConnection::Keep
                                                    : ProtoRequestFastCGI::HeaderConnection::Close;
                    request->contentLength    = -1;
                    request->headers          = Cutelyst::Headers();
                    request->connState        = ProtoRequestFastCGI::MethodLine;
                }

                memmove(request->buffer,
                        request->buffer + fcgi_all_len,
                        size_t(request->buf_size - fcgi_all_len));
                request->buf_size -= fcgi_all_len;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    return WSGI_AGAIN; // read again
}

bool ProtocolFastCGI::writeBody(ProtoRequestFastCGI *request, char *buf, qint64 len) const
{
    if (!request->body) {
        request->body = createBody(request->contentLength);
        if (!request->body) {
            return false;
        }
    }

    return request->body->write(buf, len) == len;
}

qint64 ProtocolFastCGI::readBody(Socket *sock, QIODevice *io, qint64 bytesAvailable) const
{
    auto request    = static_cast<ProtoRequestFastCGI *>(sock->protoData);
    QIODevice *body = request->body;
    int &pad        = request->buf_size;
    while (bytesAvailable && request->pktsize + pad) {
        // We need to read and ignore ending PAD data
        qint64 len = io->read(m_postBuffer,
                              qMin(m_postBufferSize, static_cast<qint64>(request->pktsize + pad)));
        if (len == -1) {
            sock->connectionClose();
            return -1;
        }
        bytesAvailable -= len;

        if (len > request->pktsize) {
            // We read past pktsize, so possibly PAD data was read too.
            pad -= len - request->pktsize;
            len              = request->pktsize;
            request->pktsize = 0;
        } else {
            request->pktsize -= len;
        }

        body->write(m_postBuffer, len);
    }

    if (request->pktsize + pad == 0) {
        request->connState = ProtoRequestFastCGI::MethodLine;
    }

    return bytesAvailable;
}

void ProtocolFastCGI::parse(Socket *sock, QIODevice *io) const
{
    // Post buffering
    auto request = static_cast<ProtoRequestFastCGI *>(sock->protoData);
    if (request->status & Cutelyst::EngineRequest::Async) {
        return;
    }

    qint64 bytesAvailable = io->bytesAvailable();
    if (request->connState == ProtoRequestFastCGI::ContentBody) {
        bytesAvailable = readBody(sock, io, bytesAvailable);
        if (bytesAvailable == -1) {
            return;
        }
    }

    do {
        qint64 len =
            io->read(request->buffer + request->buf_size, m_bufferSize - request->buf_size);
        bytesAvailable -= len;

        if (len > 0) {
            request->buf_size += len;

            if (useStats && request->startOfRequest == TimePointSteady{}) {
                request->startOfRequest = std::chrono::steady_clock::now();
            }

            if (request->buf_size < int(sizeof(struct fcgi_record))) {
                // not enough data
                continue;
            }

            int ret = processPacket(request);
            if (ret == WSGI_AGAIN) {
                continue;
            } else if (ret == WSGI_OK) {
                sock->processing++;
                if (request->body) {
                    request->body->seek(0);
                }
                sock->engine->processRequest(request);
                if (request->status & Cutelyst::EngineRequest::Async) {
                    return; // We are in async mode
                }
            } else if (ret == WSGI_BODY) {
                bytesAvailable = readBody(sock, io, bytesAvailable);
                if (bytesAvailable == -1) {
                    return;
                }
            } else {
                qCWarning(C_SERVER_FCGI) << "Failed to parse packet from"
                                         << sock->remoteAddress.toString() << sock->remotePort;
                // On error disconnect immediately
                io->close();
            }
        } else {
            qCWarning(C_SERVER_FCGI) << "Failed to read from socket" << io->errorString();
            break;
        }
    } while (bytesAvailable);
}

ProtocolData *ProtocolFastCGI::createData(Socket *sock) const
{
    return new ProtoRequestFastCGI(sock, m_bufferSize);
}

ProtoRequestFastCGI::ProtoRequestFastCGI(Socket *sock, int bufferSize)
    : ProtocolData(sock, bufferSize)
{
}

ProtoRequestFastCGI::~ProtoRequestFastCGI()
{
}

void ProtoRequestFastCGI::setupNewConnection(Socket *sock)
{
    serverAddress = sock->serverAddress;
    remoteAddress = sock->remoteAddress;
    remotePort    = sock->remotePort;
}

bool ProtoRequestFastCGI::writeHeaders(quint16 status, const Cutelyst::Headers &headers)
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
    for (const auto &[key, value] : headersData) {
        if (!hasDate && key.compare("Date", Qt::CaseInsensitive) == 0) {
            hasDate = true;
        }

        headerBuffer.append("\r\n");
        headerBuffer.append(key);
        headerBuffer.append(": ");
        headerBuffer.append(value);
    }

    if (!hasDate) {
        headerBuffer.append(static_cast<ServerEngine *>(sock->engine)->lastDate());
    }
    headerBuffer.append("\r\n\r\n", 4);

    return doWrite(headerBuffer.constData(), headerBuffer.size()) != -1;
}

qint64 ProtoRequestFastCGI::doWrite(const char *data, qint64 len)
{
    // reset for next write
    qint64 write_pos            = 0;
    quint32 proto_parser_status = 0;

    Q_FOREVER
    {
        // fastcgi packets are limited to 64k
        quint8 padding = 0;

        if (proto_parser_status == 0) {
            quint16 fcgi_len;
            if (len - write_pos < 0xffff) {
                fcgi_len = quint16(len - write_pos);
            } else {
                fcgi_len = 0xffff;
            }
            proto_parser_status = fcgi_len;

            struct fcgi_record fr;
            fr.version = FCGI_VERSION_1;
            fr.type    = FCGI_STDOUT;

            fr.req1 = quint8(stream_id >> 8);
            fr.req0 = quint8(stream_id);

            quint16 padded_len = FCGI_ALIGN(fcgi_len);
            if (padded_len > fcgi_len) {
                padding = quint8(padded_len - fcgi_len);
            }
            fr.pad = padding;

            fr.reserved = 0;
            fr.cl1      = quint8(fcgi_len >> 8);
            fr.cl0      = quint8(fcgi_len);
            if (io->write(reinterpret_cast<const char *>(&fr), sizeof(struct fcgi_record)) !=
                sizeof(struct fcgi_record)) {
                return -1;
            }
        }

        qint64 wlen = io->write(data + write_pos, proto_parser_status);
        if (padding) {
            io->write("\0\0\0\0\0\0\0\0\0", padding);
        }

        if (wlen > 0) {
            write_pos += wlen;
            proto_parser_status -= wlen;
            if (write_pos == len) {
                return write_pos;
            }
            continue;
        }
        if (wlen < 0) {
            qCWarning(C_SERVER_FCGI) << "Writing socket error" << io->errorString();
        }
        return -1;
    }
}

#define FCGI_END_REQUEST_DATA "\1\x06\0\1\0\0\0\0\1\3\0\1\0\x08\0\0\0\0\0\0\0\0\0\0"

void ProtoRequestFastCGI::processingFinished()
{
    char end_request[] = FCGI_END_REQUEST_DATA;
    const char *sid    = reinterpret_cast<char *>(&stream_id);
    // update with request id
    end_request[2]  = sid[1];
    end_request[3]  = sid[0];
    end_request[10] = sid[1];
    end_request[11] = sid[0];
    io->write(end_request, 24);

    if (!sock->requestFinished()) {
        // disconnected
        return;
    }

    if (headerConnection == ProtoRequestFastCGI::HeaderConnection::Close) {
        // Web server did not set FCGI_KEEP_CONN
        sock->connectionClose();
        return;
    }

    const auto size = buf_size;
    resetData();
    buf_size = size;

    if (status & EngineRequest::Async && buf_size) {
        sock->proto->parse(sock, io);
    }
}

#include "moc_protocolfastcgi.cpp"
