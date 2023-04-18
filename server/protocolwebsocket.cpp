/*
 * SPDX-FileCopyrightText: (C) 2017-2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "protocolwebsocket.h"

#include "protocolhttp.h"
#include "server.h"
#include "socket.h"

#include <Cutelyst/Context>
#include <Cutelyst/Headers>
#include <Cutelyst/Response>

#include <QLoggingCategory>

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
#    include <QTextCodec>
#else
#    include <QStringConverter>
#endif

Q_LOGGING_CATEGORY(CWSGI_WS, "cwsgi.websocket", QtWarningMsg)

using namespace Cutelyst;

ProtocolWebSocket::ProtocolWebSocket(Server *wsgi)
    : Protocol(wsgi)
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    , m_codec(QTextCodec::codecForName(QByteArrayLiteral("UTF-8")))
#endif
    , m_websockets_max_size(wsgi->websocketMaxSize() * 1024)
{
}

ProtocolWebSocket::~ProtocolWebSocket()
{
}

Protocol::Type ProtocolWebSocket::type() const
{
    return Protocol::Type::Http11Websocket;
}

QByteArray ProtocolWebSocket::createWebsocketHeader(quint8 opcode, quint64 len)
{
    QByteArray ret;
    ret.append(char(0x80 + opcode));

    if (len < 126) {
        ret.append(static_cast<char>(len));
    } else if (len <= static_cast<quint16>(0xffff)) {
        ret.append(char(126));

        quint8 buf[2];
        buf[1] = quint8(len & 0xff);
        buf[0] = quint8((len >> 8) & 0xff);
        ret.append(reinterpret_cast<char *>(buf), 2);
    } else {
        ret.append(127);

        quint8 buf[8];
        buf[7] = quint8(len & 0xff);
        buf[6] = quint8((len >> 8) & 0xff);
        buf[5] = quint8((len >> 16) & 0xff);
        buf[4] = quint8((len >> 24) & 0xff);
        buf[3] = quint8((len >> 32) & 0xff);
        buf[2] = quint8((len >> 40) & 0xff);
        buf[1] = quint8((len >> 48) & 0xff);
        buf[0] = quint8((len >> 56) & 0xff);
        ret.append(reinterpret_cast<char *>(buf), 8);
    }

    return ret;
}

QByteArray ProtocolWebSocket::createWebsocketCloseReply(const QString &msg, quint16 closeCode)
{
    QByteArray payload;

    const QByteArray data = msg.toUtf8().left(123);

    payload = ProtocolWebSocket::createWebsocketHeader(ProtoRequestHttp::OpCodeClose, quint64(data.size() + 2));

    quint8 buf[2];
    buf[1] = quint8(closeCode & 0xff);
    buf[0] = quint8((closeCode >> 8) & 0xff);
    payload.append(reinterpret_cast<char *>(buf), 2);

    // 125 is max payload - 2 of the above bytes
    payload.append(data);

    return payload;
}

void ProtocolWebSocket::parse(Socket *sock, QIODevice *io) const
{
    qint64 bytesAvailable = io->bytesAvailable();
    auto request          = static_cast<ProtoRequestHttp *>(sock->protoData);

    Q_FOREVER
    {
        if (!bytesAvailable ||
            !request->websocket_need ||
            (bytesAvailable < request->websocket_need && request->websocket_phase != ProtoRequestHttp::WebSocketPhasePayload)) {
            // Need more data
            return;
        }

        quint32 maxlen = qMin(request->websocket_need, static_cast<quint32>(m_postBufferSize));
        qint64 len     = io->read(m_postBuffer, maxlen);
        if (len == -1) {
            qCWarning(CWSGI_WS) << "Failed to read from socket" << io->errorString();
            sock->connectionClose();
            return;
        }
        bytesAvailable -= len;

        switch (request->websocket_phase) {
        case ProtoRequestHttp::WebSocketPhaseHeaders:
            if (!websocket_parse_header(sock, m_postBuffer, io)) {
                return;
            }
            break;
        case ProtoRequestHttp::WebSocketPhaseSize:
            if (!websocket_parse_size(sock, m_postBuffer, m_websockets_max_size)) {
                return;
            }
            break;
        case ProtoRequestHttp::WebSocketPhaseMask:
            websocket_parse_mask(sock, m_postBuffer, io);
            break;
        case ProtoRequestHttp::WebSocketPhasePayload:
            if (!websocket_parse_payload(sock, m_postBuffer, int(len), io)) {
                return;
            }
            break;
        }
    }
}

ProtocolData *ProtocolWebSocket::createData(Socket *sock) const
{
    Q_UNUSED(sock)
    return nullptr;
}

bool ProtocolWebSocket::send_text(Cutelyst::Context *c, Socket *sock, bool singleFrame) const
{
    Cutelyst::Request *request = c->request();
    auto protoRequest          = static_cast<ProtoRequestHttp *>(sock->protoData);

    const int msg_size = protoRequest->websocket_message.size();
    protoRequest->websocket_message.append(protoRequest->websocket_payload);

    QByteArray payload = protoRequest->websocket_payload;
    if (protoRequest->websocket_start_of_frame != msg_size) {
        payload = protoRequest->websocket_message.mid(protoRequest->websocket_start_of_frame);
    }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QTextCodec::ConverterState state;
    const QString frame = m_codec->toUnicode(payload.data(), payload.size(), &state);
    const bool failed   = state.invalidChars || state.remainingChars;
#else
    auto toUtf16        = QStringDecoder(QStringDecoder::Utf8);
    const QString frame = toUtf16(payload);
    const bool failed   = false; // FIXME
#endif
    if (singleFrame && (failed || (frame.isEmpty() && payload.size()))) {
        sock->connectionClose();
        return false;
    } else if (!failed) {
        protoRequest->websocket_start_of_frame = protoRequest->websocket_message.size();
        Q_EMIT request->webSocketTextFrame(frame,
                                           protoRequest->websocket_finn_opcode & 0x80,
                                           protoRequest->context);
    }

    if (protoRequest->websocket_finn_opcode & 0x80) {
        protoRequest->websocket_continue_opcode = 0;
        if (singleFrame || protoRequest->websocket_payload == protoRequest->websocket_message) {
            Q_EMIT request->webSocketTextMessage(frame, protoRequest->context);
        } else {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
            QTextCodec::ConverterState stateMsg;
            const QString msg = m_codec->toUnicode(protoRequest->websocket_message.data(), protoRequest->websocket_message.size(), &stateMsg);
            const bool failed = stateMsg.invalidChars || stateMsg.remainingChars;
#else
            auto toUtf16      = QStringDecoder(QStringDecoder::Utf8);
            const QString msg = toUtf16(protoRequest->websocket_message);
            const bool failed = false; // FIXME
#endif
            if (failed) {
                sock->connectionClose();
                return false;
            }
            Q_EMIT request->webSocketTextMessage(msg, protoRequest->context);
        }
        protoRequest->websocket_message = QByteArray();
        protoRequest->websocket_payload = QByteArray();
    }

    return true;
}

void ProtocolWebSocket::send_binary(Cutelyst::Context *c, Socket *sock, bool singleFrame) const
{
    Cutelyst::Request *request = c->request();
    auto protoRequest          = static_cast<ProtoRequestHttp *>(sock->protoData);

    protoRequest->websocket_message.append(protoRequest->websocket_payload);

    const QByteArray frame = protoRequest->websocket_payload;
    Q_EMIT request->webSocketBinaryFrame(frame,
                                         protoRequest->websocket_finn_opcode & 0x80,
                                         protoRequest->context);

    if (protoRequest->websocket_finn_opcode & 0x80) {
        protoRequest->websocket_continue_opcode = 0;
        if (singleFrame || protoRequest->websocket_payload == protoRequest->websocket_message) {
            Q_EMIT request->webSocketBinaryMessage(frame, protoRequest->context);
        } else {
            Q_EMIT request->webSocketBinaryMessage(protoRequest->websocket_message,
                                                   protoRequest->context);
        }
        protoRequest->websocket_message = QByteArray();
        protoRequest->websocket_payload = QByteArray();
    }
}

void ProtocolWebSocket::send_pong(QIODevice *io, const QByteArray data) const
{
    io->write(ProtocolWebSocket::createWebsocketHeader(ProtoRequestHttp::OpCodePong, quint64(data.size())));
    io->write(data);
}

void ProtocolWebSocket::send_closed(Cutelyst::Context *c, Socket *sock, QIODevice *io) const
{
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    quint16 closeCode = Cutelyst::Response::CloseCodeMissingStatusCode;
    QString reason;
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    QTextCodec::ConverterState state;
    const QString msg = m_codec->toUnicode(protoRequest->websocket_message.data(), protoRequest->websocket_message.size(), &state);
    const bool failed = state.invalidChars || state.remainingChars;
#else
    const bool failed = false;           // FIXME
#endif

    if (protoRequest->websocket_payload.size() >= 2) {
        closeCode = net_be16(protoRequest->websocket_payload.data());
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        reason = m_codec->toUnicode(protoRequest->websocket_payload.data() + 2, protoRequest->websocket_payload.size() - 2, &state);
#else
        auto toUtf16 = QStringDecoder(QStringDecoder::Utf8);
        reason       = toUtf16(protoRequest->websocket_payload.mid(2));
#endif
    }
    Q_EMIT c->request()->webSocketClosed(closeCode, reason);

    if (failed) {
        reason    = QString();
        closeCode = Cutelyst::Response::CloseCodeProtocolError;
    } else if (closeCode < 3000 || closeCode > 4999) {
        switch (closeCode) {
        case Cutelyst::Response::CloseCodeNormal:
        case Cutelyst::Response::CloseCodeGoingAway:
        case Cutelyst::Response::CloseCodeProtocolError:
        case Cutelyst::Response::CloseCodeDatatypeNotSupported:
            //    case Cutelyst::Response::CloseCodeReserved1004:
            break;
        case Cutelyst::Response::CloseCodeMissingStatusCode:
            if (protoRequest->websocket_payload.isEmpty()) {
                closeCode = Cutelyst::Response::CloseCodeNormal;
            } else {
                closeCode = Cutelyst::Response::CloseCodeProtocolError;
            }
            break;
            //    case Cutelyst::Response::CloseCodeAbnormalDisconnection:
        case Cutelyst::Response::CloseCodeWrongDatatype:
        case Cutelyst::Response::CloseCodePolicyViolated:
        case Cutelyst::Response::CloseCodeTooMuchData:
        case Cutelyst::Response::CloseCodeMissingExtension:
        case Cutelyst::Response::CloseCodeBadOperation:
            //    case Cutelyst::Response::CloseCodeTlsHandshakeFailed:
            break;
        default:
            reason    = QString();
            closeCode = Cutelyst::Response::CloseCodeProtocolError;
            break;
        }
    }

    const QByteArray reply = ProtocolWebSocket::createWebsocketCloseReply(reason, closeCode);
    io->write(reply);

    sock->connectionClose();
}

bool ProtocolWebSocket::websocket_parse_header(Socket *sock, const char *buf, QIODevice *io) const
{
    const char byte1 = buf[0];
    const char byte2 = buf[1];

    auto protoRequest                    = static_cast<ProtoRequestHttp *>(sock->protoData);
    protoRequest->websocket_finn_opcode  = quint8(byte1);
    protoRequest->websocket_payload_size = byte2 & 0x7f;

    quint8 opcode = byte1 & 0xf;

    bool websocket_has_mask = byte2 >> 7;
    if (!websocket_has_mask ||
        ((opcode == ProtoRequestHttp::OpCodePing || opcode == ProtoRequestHttp::OpCodeClose) && protoRequest->websocket_payload_size > 125) ||
        (byte1 & 0x70) ||
        ((opcode >= ProtoRequestHttp::OpCodeReserved3 && opcode <= ProtoRequestHttp::OpCodeReserved7) ||
         (opcode >= ProtoRequestHttp::OpCodeReservedB && opcode <= ProtoRequestHttp::OpCodeReservedF)) ||
        (!(byte1 & 0x80) && opcode != ProtoRequestHttp::OpCodeText && opcode != ProtoRequestHttp::OpCodeBinary && opcode != ProtoRequestHttp::OpCodeContinue) ||
        (protoRequest->websocket_continue_opcode && (opcode == ProtoRequestHttp::OpCodeText || opcode == ProtoRequestHttp::OpCodeBinary))) {
        // RFC errors
        // client to server MUST have a mask
        // Control opcode cannot have payload bigger than 125
        // RSV bytes MUST not be set
        // reserved opcodes must not be set 3-7
        // reserved opcodes must not be set B-F
        // Only Text/Bynary/Coninue opcodes can be fragmented
        // Continue opcode was set but was NOT followed by CONTINUE

        io->write(ProtocolWebSocket::createWebsocketCloseReply(QString(), 1002)); // Protocol error
        sock->connectionClose();
        return false;
    }

    if (opcode == ProtoRequestHttp::OpCodeText || opcode == ProtoRequestHttp::OpCodeBinary) {
        protoRequest->websocket_message        = QByteArray();
        protoRequest->websocket_start_of_frame = 0;
        if (!(byte1 & 0x80)) {
            // FINN byte not set, store opcode for continue
            protoRequest->websocket_continue_opcode = opcode;
        }
    }

    if (protoRequest->websocket_payload_size == 126) {
        protoRequest->websocket_need  = 2;
        protoRequest->websocket_phase = ProtoRequestHttp::WebSocketPhaseSize;
    } else if (protoRequest->websocket_payload_size == 127) {
        protoRequest->websocket_need  = 8;
        protoRequest->websocket_phase = ProtoRequestHttp::WebSocketPhaseSize;
    } else {
        protoRequest->websocket_need  = 4;
        protoRequest->websocket_phase = ProtoRequestHttp::WebSocketPhaseMask;
    }

    return true;
}

bool ProtocolWebSocket::websocket_parse_size(Socket *sock, const char *buf, int websockets_max_message_size) const
{
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    quint64 size;
    if (protoRequest->websocket_payload_size == 126) {
        size = net_be16(buf);
    } else if (protoRequest->websocket_payload_size == 127) {
        size = net_be64(buf);
    } else {
        qCCritical(CWSGI_WS) << "BUG error in websocket parser:" << protoRequest->websocket_payload_size;
        sock->connectionClose();
        return false;
    }

    if (size > static_cast<quint64>(websockets_max_message_size)) {
        qCCritical(CWSGI_WS) << "Payload size too big" << size << "max allowed" << websockets_max_message_size;
        sock->connectionClose();
        return false;
    }
    protoRequest->websocket_payload_size = size;

    protoRequest->websocket_need  = 4;
    protoRequest->websocket_phase = ProtoRequestHttp::WebSocketPhaseMask;

    return true;
}

void ProtocolWebSocket::websocket_parse_mask(Socket *sock, char *buf, QIODevice *io) const
{
    auto ptr                     = reinterpret_cast<const quint32 *>(buf);
    auto protoRequest            = static_cast<ProtoRequestHttp *>(sock->protoData);
    protoRequest->websocket_mask = *ptr;

    protoRequest->websocket_phase = ProtoRequestHttp::WebSocketPhasePayload;
    protoRequest->websocket_need  = quint32(protoRequest->websocket_payload_size);

    protoRequest->websocket_payload = QByteArray();
    if (protoRequest->websocket_payload_size == 0) {
        websocket_parse_payload(sock, buf, 0, io);
    } else {
        protoRequest->websocket_payload.reserve(int(protoRequest->websocket_payload_size));
    }
}

bool ProtocolWebSocket::websocket_parse_payload(Socket *sock, char *buf, int len, QIODevice *io) const
{
    auto protoRequest = static_cast<ProtoRequestHttp *>(sock->protoData);
    auto mask         = reinterpret_cast<quint8 *>(&protoRequest->websocket_mask);
    for (int i = 0, maskIx = protoRequest->websocket_payload.size(); i < len; ++i, ++maskIx) {
        buf[i] = buf[i] ^ mask[maskIx % 4];
    }

    protoRequest->websocket_payload.append(buf, len);
    if (quint64(protoRequest->websocket_payload.size()) < protoRequest->websocket_payload_size) {
        // need more data
        protoRequest->websocket_need -= uint(len);
        return true;
    }

    protoRequest->websocket_need  = 2;
    protoRequest->websocket_phase = ProtoRequestHttp::WebSocketPhaseHeaders;

    Cutelyst::Request *request = protoRequest->context->request();

    switch (protoRequest->websocket_finn_opcode & 0xf) {
    case ProtoRequestHttp::OpCodeContinue:
        switch (protoRequest->websocket_continue_opcode) {
        case ProtoRequestHttp::OpCodeText:
            if (!send_text(protoRequest->context, sock, false)) {
                return false;
            }
            break;
        case ProtoRequestHttp::OpCodeBinary:
            send_binary(protoRequest->context, sock, false);
            break;
        default:
            qCCritical(CWSGI_WS) << "Invalid CONTINUE opcode:" << (protoRequest->websocket_finn_opcode & 0xf);
            sock->connectionClose();
            return false;
        }
        break;
    case ProtoRequestHttp::OpCodeText:
        if (!send_text(protoRequest->context, sock, protoRequest->websocket_finn_opcode & 0x80)) {
            return false;
        }
        break;
    case ProtoRequestHttp::OpCodeBinary:
        send_binary(protoRequest->context, sock, protoRequest->websocket_finn_opcode & 0x80);
        break;
    case ProtoRequestHttp::OpCodeClose:
        send_closed(protoRequest->context, sock, io);
        return false;
    case ProtoRequestHttp::OpCodePing:
        send_pong(io, protoRequest->websocket_payload.left(125));
        sock->flush();
        break;
    case ProtoRequestHttp::OpCodePong:
        Q_EMIT request->webSocketPong(protoRequest->websocket_payload,
                                      protoRequest->context);
        break;
    default:
        break;
    }

    return true;
}
