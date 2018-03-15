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

unsigned char* hpackDecodeString(unsigned char *src, unsigned char *src_end, QString &value, int &error);

unsigned char* hpackDecodeInt(unsigned char *src, unsigned char *src_end, qint32 &value, quint8 prefix_max, int &errorno)
{
    quint8 mult;

    if (src == src_end) {
        qDebug() << "bug";
        errorno = -2;
        value = -1;
        return src;
    }

    src++;
    qDebug() << "in " << prefix_max << ((quint8)*src & prefix_max) << (quint8(*src) & prefix_max);
    if ((value = (quint8)*src & prefix_max) == prefix_max) {
        mult = 0;
        qDebug() << "in hpackDecodeInt";

        while (src < src_end) {

            value += (*src & 0x7f) << mult;

            if (value > UINT16_MAX) {
                errorno = -3;
                value = -1;
                return src;
            }

            if ((*src++ & 0x80) == 0) {
                errorno = 0;
                return src;
            }

            mult += 7;

            if (mult >= 32) // we only allow at most 4 octets (excluding prefix) to be used as int (== 2**(4*7) == 2**28)
            {
                errorno = -3; // Decoding of an integer gives a value too large

                value = -1;
                return src;
            }

        }
    }

    qDebug() << "bug end";
    errorno = -2;
    value = -1;

    return src;
}

// This decodes an UInt
// it returns nullptr if it tries to read past end
// The value can overflow it's capacity, which should be harmless as it would
// give parsing errors on other parts
unsigned char *decodeUInt16(unsigned char *src, unsigned char *src_end, quint16 &dst, quint8 mask)
{
    qDebug() << Q_FUNC_INFO << "value 0" << QByteArray((char*)src, 10).toHex();

    dst = *src & mask;
    if (dst == mask) {
        int M = 0;
        do {
            if (++src >= src_end) {
                dst = -1;
                return nullptr;
            }

            dst += (*src & 0x7f) << M;
            M += 7;
        } while (*src & 0x80);
    }

    return ++src;
}

quint64 encode_int(quint8 *dst, quint32 I, quint8 N)
{
    quint16 twoN = (1 << N) -1;
    if (I < twoN) {
        *dst = I;
        return 1;
    }

    I -= twoN;
    *dst = twoN;
    quint64 i = 1;
    for (; I >= 128; i++) {
        *(dst + i) = (I & 0x7f) | 0x80;
        I = I >> 7;
    }
    *(dst + (i++)) = I;

    return i;
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
        qDebug() << "HUFFMAN value" << str_len /*<< QByteArray(reinterpret_cast<const char *>(buf + len), str_len).toHex()*/;
        int errorno = 0;
        buf = hpackDecodeString(buf, buf + str_len, dst, errorno);
        if (!buf) {
            return nullptr;
        }
        qDebug() << "HUFFMAN decoded" << dst << errorno;
    } else {
        qDebug() << "Not HUFFMAN  decoded" << buf << str_len << (buf + str_len) << itEnd;
        if (buf + str_len <= itEnd) {
            itEnd = buf + str_len;

            while (buf < itEnd) {
                dst += QLatin1Char(*(buf++));
            }
        } else {
            qDebug() << "Not HUFFMAN value decoded  error";
            return nullptr;
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
        qDebug() << "HUFFMAN key" << str_len;
        int errorno = 0;
        buf = hpackDecodeString(buf, buf + str_len, dst, errorno);
        if (!buf) {
            return nullptr;
        }
        qDebug() << "HUFFMAN decoded" << dst << errorno;
    } else {
        qDebug() << "Not HUFFMAN  decoded" << buf << str_len << (buf + str_len) << itEnd;
        if (buf + str_len <= itEnd) {
            itEnd = buf + str_len;

            while (buf < itEnd) {
                QChar c = QLatin1Char(*(buf++));
                if (c.isUpper()) {
                    qDebug() << "Not HUFFMAN  decoded upper error";
                    return nullptr;
                }
                dst += c;
            }
        } else {
            qDebug() << "Not HUFFMAN  decoded out bound error";
            return nullptr;
        }
    }
    return buf;
}

HPack::HPack(int maxTableSize) : m_currentMaxDynamicTableSize(maxTableSize), m_maxTableSize(maxTableSize)
{
//    QByteArray bug("\xff\xff\xff\xff\xff\x7f");
    QByteArray bug("\xff\x80\x80\x80\x80\x80\x80\x80\x80\x80\x01\x80\x7e");
    qDebug() << "+_+_+_+_+_+_++"  << bug.toHex() << (quint8*)(bug.begin()) << (quint8*)(bug.end());
    qDebug() << "+_+_+_+_+_+_++"  << bug.toHex() << (quint8*)(bug.data()) << bug.end();
    quint16 dst = 0;
    quint8 *next = decodeUInt16(reinterpret_cast<unsigned char *>(bug.begin()), reinterpret_cast<unsigned char *>(bug.end()), dst, INT_MASK(7));
    qDebug() << "next"  << dst << next;

}

HPack::~HPack()
{

}

void HPack::encodeStatus(int status)
{
    if (status == 200) {
        buf.append(0x88);
    } else if (status == 204) {
        buf.append(0x89);
    } else if (status == 206) {
        buf.append(0x8A);
    } else if (status == 304) {
        buf.append(0x8B);
    } else if (status == 400) {
        buf.append(0x8C);
    } else if (status == 404) {
        buf.append(0x8D);
    } else if (status == 500) {
        buf.append(0x8E);
    } else {
        encodeHeader(QByteArrayLiteral(":status"), QByteArray::number(status));
    }
}

void HPack::encodeHeader(const QByteArray &key, const QByteArray &value)
{

}

QByteArray HPack::data() const
{
    return buf;
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
    qDebug() << "validPseudoHeader" << k << v << stream->path << stream->method << stream->authority << stream->scheme;
    if (k == QLatin1String(":path")) {
        if (stream->path.isEmpty() && !v.isEmpty()) {
            stream->path = v;
            return true;
        }
    } else if (k == QLatin1String(":method")) {
        if (stream->method.isEmpty()) {
            stream->method = v;
            return true;
        }
    } else if (k == QLatin1String(":authority")) {
        stream->authority = v;
        return true;
    } else if (k == QLatin1String(":scheme")) {
        if (stream->scheme.isEmpty()) {
            stream->scheme = v;
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
        int errorno(0);
        quint16 intValue(0);
        qDebug() << "decode LOOP" << it;
        if (*it & 0x80){
            it = decodeUInt16(it, itEnd, intValue, INT_MASK(7));
            qDebug() << "6.1 Indexed Header Field Representation" << *it << intValue << errorno << it;
            if (!it || intValue <= 0) {
                return ErrorCompressionError;
            }

            QString key;
            QString value;
            if (intValue > 61) {
                qDebug() << "6.1 Indexed Header Field Representation dynamic table lookup" << *it << intValue << m_dynamicTable.size();
                intValue -= 62;
                if (intValue < qint64(m_dynamicTable.size())) {
                    const auto h = m_dynamicTable[intValue];
                    key = h.key;
                    value = h.value;
                    qDebug() << "=========================GETTING from dynamic table key/value" << key << value;
                } else {
                    qDebug() << "=========================FAILED GETTING from dynamic table key/value" << key << value;
                    return ErrorCompressionError;
                }
            } else  {
                const auto h = hpackStaticHeaders[intValue];
                key = h.key;
                value = h.value;
            }

            qDebug() << "header" << key << value;
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
                qDebug() << "6.2.1 Literal Header Field" << *it << "value" << intValue << "allowedToUpdate" << allowedToUpdate;
            } else if (*it & 0x20) {
                it = decodeUInt16(it, itEnd, intValue, INT_MASK(5));
                qDebug() << "6.3 Dynamic Table update" << *it << "value" << intValue << "allowedToUpdate" << allowedToUpdate << m_maxTableSize;
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
                const auto h = hpackStaticHeaders[intValue];
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
                qDebug() << "=========================parsing string error";
                return ErrorCompressionError;
            }

            if (key.startsWith(QLatin1Char(':'))) {
                if (!pseudoHeadersAllowed || !validPseudoHeader(key, value, stream)) {
                    qDebug() << "=========================not valid header 1" << pseudoHeadersAllowed << key << value;
                    return ErrorProtocolError;
                }
            } else {
                if (!validHeader(key, value)) {
                    qDebug() << "=========================not valid header 2" << key << value;
                    return ErrorProtocolError;
                }
                pseudoHeadersAllowed = false;
                consumeHeader(key, value, stream);
                stream->headers.pushHeader(key, value);
            }

            if (addToDynamicTable) {
                const int size = key.length() + value.length() + 32;
                qDebug() << "=========================Adding to dynamic table key/value" << size << m_currentMaxDynamicTableSize << key;
                while (size + m_dynamicTableSize > m_currentMaxDynamicTableSize && !m_dynamicTable.empty()) {
                    auto it = m_dynamicTable.takeLast();
                    qDebug() << "=========================Remove ONE dynamic table key/value" << it.key;
                    m_dynamicTableSize -= it.key.length() + it.value.length() + 32;
                }

                qDebug() << "=========================CAN ADD ONE dynamic table key/value" << size << m_dynamicTableSize << (size + m_dynamicTableSize) << m_currentMaxDynamicTableSize;
                if (size + m_dynamicTableSize <= m_currentMaxDynamicTableSize) {
                    m_dynamicTable.prepend({ key, value });
                    m_dynamicTableSize += size;
                    qDebug() << "=========================ADDED to dyn table" << size << m_dynamicTableSize;
                }
            }

            qDebug() << "header key/value" << key << value;
        }

        allowedToUpdate = false;
    }

    if (stream->path.isEmpty() || stream->method.isEmpty() || stream->scheme.isEmpty()) {
        return ErrorProtocolError;
    }

    return 0;
}

unsigned char* hpackDecodeString(unsigned char *src, unsigned char *src_end, QString &value, int &error)
{
      uint8_t state = 0;
      const HPackPrivate::HuffDecode *entry = nullptr;
      QByteArray result/*(len * 2)*/; // max compression ratio is >= 0.5
//      char *dst = result.data();

      do {
          if (entry) {
              state = entry->state;
          }
          entry = HPackPrivate::huff_decode_table[state] + (*src >> 4);

          if ((entry->flags & HPackPrivate::HUFF_FAIL) != 0)
          {
#     ifdef DEBUG
              hpack_errno = -6; // A decoder decoded an invalid Huffman sequence
#     endif

              error = -6;
              return nullptr;
          }

          if ((entry->flags & HPackPrivate::HUFF_SYM) != 0) {
              //          *dst++ = entry->sym;
              result.append(entry->sym);
          }

          entry = HPackPrivate::huff_decode_table[entry->state] + (*src & 0x0f);

          if ((entry->flags & HPackPrivate::HUFF_FAIL) != 0)
          {
#     ifdef DEBUG
              hpack_errno = -6; // A decoder decoded an invalid Huffman sequence
#     endif

              error = -6;
              return nullptr;
          }

          if ((entry->flags & HPackPrivate::HUFF_SYM) != 0) {
              result.append(entry->sym);
              //          *dst++ = entry->sym;
          }

      } while (++src < src_end);

//          if (++src < src_end)
//          {
//              state = entry->state;

//              goto loop;
//          }

      qDebug() << "maybe_eos = " << ((entry->flags & HPackPrivate::HUFF_ACCEPTED) != 0) << "entry->state =" << entry->state;

              if ((entry->flags & HPackPrivate::HUFF_ACCEPTED) == 0)
      {
#     ifdef DEBUG
          hpack_errno = (entry->state == 28 ? -7   // A invalid header name or value character was coded
                                            : -6); // A decoder decoded an invalid Huffman sequence
#     endif

          error = -7;
          return nullptr;
      }

//      result.size_adjust(dst);

      value = QString::fromLatin1(result);
      return src_end;
}
