/*
 * Copyright (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
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
#include "hpack.h"
#include "hpack_p.h"

#include "protocolhttp2.h"

#include <vector>

#include <QDebug>

#define INT_MASK(bits) (1 << bits) - 1

using namespace CWSGI;

unsigned char* hpackDecodeString(unsigned char *src, unsigned char *src_end, QString &value, int len);

// This decodes an UInt
// it returns nullptr if it tries to read past end
// The value can overflow it's capacity, which should be harmless as it would
// give parsing errors on other parts
unsigned char *decodeUInt16(unsigned char *src, unsigned char *src_end, quint16 &dst, quint8 mask)
{
//    qDebug() << Q_FUNC_INFO << "value 0" << QByteArray((char*)src, 10).toHex();

    dst = *src & mask;
    if (dst == mask) {
        int M = 0;
        do {
            if (++src >= src_end) {
                dst = quint16(-1);
                return nullptr;
            }

            dst += (*src & 0x7f) << M;
            M += 7;
        } while (*src & 0x80);
    }

    return ++src;
}

void encodeUInt16(QByteArray &buf, int I, quint8 mask)
{
    if (I < mask) {
        buf.append(char(I));
        return;
    }

    I -= mask;
    buf.append(char(mask));
    while (I >= 128) {
        buf.append(char((I & 0x7f) | 0x80));
        I = I >> 7;
    }
    buf.append(char(I));
}

static inline void encodeH2caseHeader(QByteArray &buf, const QString &key) {

    encodeUInt16(buf, key.length(), INT_MASK(7));
    for (auto keyIt : key) {
        if (keyIt.isLetter()) {
            buf.append(keyIt.toLower().toLatin1());
        } else if (keyIt == QLatin1Char('_')) {
            buf.append('-');
        } else {
            buf.append(keyIt.toLatin1());
        }
    }
}

unsigned char *parse_string(QString &dst, unsigned char *buf, quint8 *itEnd)
{
    quint16 str_len = 0;

    bool huffmanDecode = *buf & 0x80;

    buf = decodeUInt16(buf, itEnd, str_len, INT_MASK(7));
    if (!buf) {
        return nullptr;
    }

    if (huffmanDecode) {
        buf = hpackDecodeString(buf, buf + str_len, dst, str_len);
        if (!buf) {
            return nullptr;
        }
    } else {
        if (buf + str_len <= itEnd) {
            dst = QString::fromLatin1(reinterpret_cast<const char *>(buf), str_len);
            buf += str_len;
        } else {
            return nullptr; // Reading past end
        }
    }
    return buf;
}

unsigned char *parse_string_key(QString &dst, quint8 *buf, quint8 *itEnd)
{
    quint16 str_len = 0;
    bool huffmanDecode = *buf & 0x80;

    buf = decodeUInt16(buf, itEnd, str_len, INT_MASK(7));
    if (!buf) {
        return nullptr;
    }

    if (huffmanDecode) {
        buf = hpackDecodeString(buf, buf + str_len, dst, str_len);
        if (!buf) {
            return nullptr;
        }
    } else {
        if (buf + str_len <= itEnd) {
            itEnd = buf + str_len;

            while (buf < itEnd) {
                QChar c = QLatin1Char(char(*(buf++)));
                if (c.isUpper()) {
                    return nullptr;
                }
                dst += c;
            }
        } else {
            return nullptr; // Reading past end
        }
    }
    return buf;
}

HPack::HPack(int maxTableSize) : m_currentMaxDynamicTableSize(maxTableSize), m_maxTableSize(maxTableSize)
{

}

HPack::~HPack()
{

}

void HPack::encodeHeaders(int status, const QHash<QString, QString> &headers, QByteArray &buf, CWsgiEngine *engine)
{
    if (status == 200) {
        buf.append(char(0x88));
    } else if (status == 204) {
        buf.append(char(0x89));
    } else if (status == 206) {
        buf.append(char(0x8A));
    } else if (status == 304) {
        buf.append(char(0x8B));
    } else if (status == 400) {
        buf.append(char(0x8C));
    } else if (status == 404) {
        buf.append(char(0x8D));
    } else if (status == 500) {
        buf.append(char(0x8E));
    } else {
        buf.append(char(0x08));

        const QByteArray statusStr = QByteArray::number(status);
        encodeUInt16(buf, statusStr.length(), INT_MASK(4));
        buf.append(statusStr);
    }

    bool hasDate = false;
    auto it = headers.constBegin();
    while (it != headers.constEnd()) {
        const QString &key = it.key();
        const QString &value = it.value();
        if (!hasDate && key == QLatin1String("DATE")) {
            hasDate = true;
        }

        auto staticIt = HPackPrivate::hpackStaticHeadersCode.constFind(key);
        if (staticIt != HPackPrivate::hpackStaticHeadersCode.constEnd()) {
            buf.append(staticIt.value(), 2);

            encodeUInt16(buf, value.length(), INT_MASK(7));
            buf.append(value.toLatin1());
        } else {
            buf.append('\x00');
            encodeH2caseHeader(buf, key);

            encodeUInt16(buf, value.length(), INT_MASK(7));
            buf.append(value.toLatin1());
        }

        ++it;
    }

    if (!hasDate) {
        const QByteArray date = engine->lastDate().mid(8);
        if (date.length() != 29) {
            // This should never happen but...
            return;
        }

        // 0f12 Date header not indexed
        // 1d = date length: 29
        buf.append("\x0f\x12\x1d", 3);
        buf.append(date);
    }
}

enum ErrorCodes {
    ErrorNoError = 0x0,
    ErrorProtocolError = 0x1,
    ErrorInternalError = 0x2,
    ErrorFlowControlError = 0x3,
    ErrorSettingsTimeout = 0x4,
    ErrorStreamClosed = 0x5,
    ErrorFrameSizeError = 0x6,
    ErrorRefusedStream = 0x7,
    ErrorCancel = 0x8,
    ErrorCompressionError = 0x9,
    ErrorConnectError = 0xA,
    ErrorEnhanceYourCalm  = 0xB,
    ErrorInadequateSecurity = 0xC,
    ErrorHttp11Required = 0xD
};

inline bool validPseudoHeader(const QString &k, const QString &v, H2Stream *stream)
{
//    qDebug() << "validPseudoHeader" << k << v << stream->path << stream->method << stream->scheme;
    if (k == QLatin1String(":path")) {
        if (!stream->gotPath && !v.isEmpty()) {
            int leadingSlash = 0;
            while (v[leadingSlash] == QLatin1Char('/')) {
                ++leadingSlash;
            }

            int pos = v.indexOf(QLatin1Char('?'));
            if (pos == -1) {
                stream->setPath(v.mid(leadingSlash));
            } else {
                stream->setPath(v.mid(leadingSlash, pos - leadingSlash));
                stream->query = v.mid(++pos).toLatin1();
            }
            stream->gotPath = true;
            return true;
        }
    } else if (k == QLatin1String(":method")) {
        if (stream->method.isEmpty()) {
            stream->method = v;
            return true;
        }
    } else if (k == QLatin1String(":authority")) {
        stream->serverAddress = v;
        return true;
    } else if (k == QLatin1String(":scheme")) {
        if (stream->scheme.isEmpty()) {
            stream->scheme = v;
            stream->isSecure = v == QLatin1String("https");
            return true;
        }
    }
    return false;
}

inline bool validHeader(const QString &k, const QString &v)
{
    return k != QLatin1String("connection") &&
            (k != QLatin1String("te") || v == QLatin1String("trailers"));
}

inline void consumeHeader(const QString &k, const QString &v, H2Stream *stream)
{
    if (k == QLatin1String("content-length")) {
        stream->contentLength = v.toLongLong();
    }
}

int HPack::decode(unsigned char *it, unsigned char *itEnd, H2Stream *stream)
{
    bool pseudoHeadersAllowed = true;
    bool allowedToUpdate = true;
    while (it < itEnd) {
        quint16 intValue(0);
        if (*it & 0x80){
            it = decodeUInt16(it, itEnd, intValue, INT_MASK(7));
//            qDebug() << "6.1 Indexed Header Field Representation" << *it << intValue << it;
            if (!it || intValue == 0) {
                return ErrorCompressionError;
            }

            QString key;
            QString value;
            if (intValue > 61) {
//                qDebug() << "6.1 Indexed Header Field Representation dynamic table lookup" << *it << intValue << m_dynamicTable.size();
                intValue -= 62;
                if (intValue < qint64(m_dynamicTable.size())) {
                    const auto h = m_dynamicTable[intValue];
                    key = h.key;
                    value = h.value;
                } else {
                    return ErrorCompressionError;
                }
            } else  {
                const auto h = HPackPrivate::hpackStaticHeaders[intValue];
                key = h.key;
                value = h.value;
            }

//            qDebug() << "header" << key << value;
            if (key.startsWith(QLatin1Char(':'))) {
                if (!pseudoHeadersAllowed || !validPseudoHeader(key, value, stream)) {
                    return ErrorProtocolError;
                }
            } else {
                if (!validHeader(key, value)) {
                    return ErrorProtocolError;
                }
                pseudoHeadersAllowed = false;
                consumeHeader(key, value, stream);
                stream->headers.pushHeader(key, value);
            }
        } else {
            bool addToDynamicTable = false;
            if (*it & 0x40) {
                // 6.2.1 Literal Header Field with Incremental Indexing
                it = decodeUInt16(it, itEnd, intValue, INT_MASK(6));
                if (!it) {
                    return ErrorCompressionError;
                }
                addToDynamicTable = true;
//                qDebug() << "6.2.1 Literal Header Field" << *it << "value" << intValue << "allowedToUpdate" << allowedToUpdate;
            } else if (*it & 0x20) {
                it = decodeUInt16(it, itEnd, intValue, INT_MASK(5));
//                qDebug() << "6.3 Dynamic Table update" << *it << "value" << intValue << "allowedToUpdate" << allowedToUpdate << m_maxTableSize;
                if (!it || intValue > m_maxTableSize || !allowedToUpdate) {
                    return ErrorCompressionError;
                }

                m_currentMaxDynamicTableSize = intValue;
                while (m_dynamicTableSize > m_currentMaxDynamicTableSize && !m_dynamicTable.empty()) {
                    auto header = m_dynamicTable.takeLast();
                    m_dynamicTableSize -= header.key.length() + header.value.length() + 32;
                }

                continue;
            } else {
                // 6.2.2 Literal Header Field without Indexing
                // 6.2.3 Literal Header Field Never Indexed
                it = decodeUInt16(it, itEnd, intValue, INT_MASK(4));
                if (!it) {
                    return ErrorCompressionError;
                }
            }

            if (intValue > 61) {
                return ErrorCompressionError;
            }

            QString key;
            if (intValue != 0) {
                const auto h = HPackPrivate::hpackStaticHeaders[intValue];
                key = h.key;
            } else {
                it = parse_string_key(key, it, itEnd);
                if (!it) {
                    return ErrorProtocolError;
                }
            }

            QString value;
            it = parse_string(value, it, itEnd);
            if (!it) {
                return ErrorCompressionError;
            }

            if (key.startsWith(QLatin1Char(':'))) {
                if (!pseudoHeadersAllowed || !validPseudoHeader(key, value, stream)) {
                    return ErrorProtocolError;
                }
            } else {
                if (!validHeader(key, value)) {
                    return ErrorProtocolError;
                }
                pseudoHeadersAllowed = false;
                consumeHeader(key, value, stream);
                stream->headers.pushHeader(key, value);
            }

            if (addToDynamicTable) {
                const int size = key.length() + value.length() + 32;
                while (size + m_dynamicTableSize > m_currentMaxDynamicTableSize && !m_dynamicTable.empty()) {
                    const DynamicTableEntry entry = m_dynamicTable.takeLast();
                    m_dynamicTableSize -= entry.key.length() + entry.value.length() + 32;
                }

                if (size + m_dynamicTableSize <= m_currentMaxDynamicTableSize) {
                    m_dynamicTable.prepend({ key, value });
                    m_dynamicTableSize += size;
                }
            }

//            qDebug() << "header key/value" << key << value;
        }

        allowedToUpdate = false;
    }

    if (!stream->gotPath || stream->method.isEmpty() || stream->scheme.isEmpty()) {
        return ErrorProtocolError;
    }

    return 0;
}

unsigned char* hpackDecodeString(unsigned char *src, unsigned char *src_end, QString &value, int len)
{
      quint8 state = 0;
      const HPackPrivate::HuffDecode *entry = nullptr;
      value.reserve(len * 2); // max compression ratio is >= 0.5

      do {
          if (entry) {
              state = entry->state;
          }
          entry = HPackPrivate::huff_decode_table[state] + (*src >> 4);

          if (entry->flags & HPackPrivate::HUFF_FAIL) {
              // A decoder decoded an invalid Huffman sequence
              return nullptr;
          }

          if (entry->flags & HPackPrivate::HUFF_SYM) {
              value.append(QLatin1Char(char(entry->sym)));
          }

          entry = HPackPrivate::huff_decode_table[entry->state] + (*src & 0x0f);

          if (entry->flags & HPackPrivate::HUFF_FAIL) {
              // A decoder decoded an invalid Huffman sequence
              return nullptr;
          }

          if ((entry->flags & HPackPrivate::HUFF_SYM) != 0) {
              value.append(QLatin1Char(char(entry->sym)));
          }

      } while (++src < src_end);

//      qDebug() << "maybe_eos = " << ((entry->flags & HPackPrivate::HUFF_ACCEPTED) != 0) << "entry->state =" << entry->state;

      if ((entry->flags & HPackPrivate::HUFF_ACCEPTED) == 0) {
          // entry->state == 28 // A invalid header name or value character was coded
          // entry->state != 28 // A decoder decoded an invalid Huffman sequence
          return nullptr;
      }

      return src_end;
}
