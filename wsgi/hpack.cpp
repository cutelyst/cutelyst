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

using namespace CWSGI;

quint64 decode_int(quint32 &dst, const quint8 *buf, quint8 N)
{
    quint64 len = 1;
    quint16 twoN = (1 << N) -1;
    dst = *buf & twoN;
    if (dst == twoN) {
        int M = 0;
        do {
            dst += (*(buf + len) & 0x7f) << M;
            M += 7;
        } while (*(buf + (len++)) & 0x80);
    }

    return len;
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

quint64 parse_string(HuffmanTree *huffman, QString &dst, const quint8 *buf, const quint8 *itEnd, bool &error)
{
    quint32 str_len = 0;
    quint64 len = decode_int(str_len, buf, 7);
    if (*buf & 0x80) {
        qDebug() << "HUFFMAN value" << len << str_len << (buf + len + str_len < itEnd) << (buf + len + str_len) << itEnd/*<< QByteArray(reinterpret_cast<const char *>(buf + len), str_len).toHex()*/;
        dst = huffman->decode(buf + len, str_len, error);
    } else {
        for (uint i = 0; i < str_len; i++) {
            dst += QLatin1Char(*(buf + (len + i)));
        }
    }
    return len + str_len;
}

quint64 parse_string_key(HuffmanTree *huffman, QString &dst, const quint8 *buf, const quint8 *itEnd, bool &error)
{
    quint32 str_len = 0;
    quint64 len = decode_int(str_len, buf, 7);
    if ((*buf & 0x80) > 0) {
        qDebug() << "HUFFMAN key" << len << str_len;
        dst = huffman->decode(buf+len, str_len, error);
    } else {
        for (uint i = 0; i < str_len; i++) {
            QChar c = QLatin1Char(*(buf + (len + i)));
            if (c.isUpper()) {
                error = true;
                return 0;
            }
            dst += c;
        }
    }
    return len + str_len;
}

HPack::HPack(int maxTableSize) : m_currentMaxDynamicTableSize(maxTableSize), m_maxTableSize(maxTableSize)
{
    m_huffmanTree = new HuffmanTree();
}

HPack::~HPack()
{
    delete m_huffmanTree;
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

int HPack::decode(const quint8 *it, const quint8 *itEnd, H2Stream *stream)
{
    bool pseudoHeadersAllowed = true;
    bool allowedToUpdate = true;
    while (it < itEnd) {
        quint32 intValue(0);
        quint64 len(0);
        if (*it & 0x80){
            len = decode_int(intValue, it, 7);
            qDebug() << "6.1 Indexed Header Field Representation" << *it << intValue << len;
            if (intValue == 0) {
                return ErrorCompressionError;
            }

            QString key;
            QString value;
            if (intValue > 61) {
                qDebug() << "6.1 Indexed Header Field Representation dynamic table lookup" << *it << intValue << len << m_dynamicTable.size();
                intValue -= 62;
                if (intValue < m_dynamicTable.size()) {
                    auto h = m_dynamicTable.at(intValue);
                    key = h.key;
                    value = h.value;
                    qDebug() << "=========================GETTING from dynamic table key/value" << key << value;
                } else {
                    return ErrorCompressionError;
                }
            } else  {
                auto h = hpackStaticHeaders[intValue];
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
            it += len;

        } else {
            bool addToDynamicTable = false;
            if (*it & 0x40) {
                // 6.2.1 Literal Header Field with Incremental Indexing
                len = decode_int(intValue, it, 6);
                addToDynamicTable = true;
                qDebug() << "6.2.1 Literal Header Field" << *it << "value" << intValue << len << "allowedToUpdate" << allowedToUpdate;
            } else if (*it & 0x20) {
                len = decode_int(intValue, it, 5);
                qDebug() << "6.3 Dynamic Table update" << *it << "value" << intValue << len << "allowedToUpdate" << allowedToUpdate << m_maxTableSize;
                if (intValue > m_maxTableSize) {
                    qDebug() << "Trying to update beyond limits";
                    return ErrorCompressionError;
                }
                if (!allowedToUpdate) {
                    return ErrorCompressionError;
                }
                m_currentMaxDynamicTableSize = intValue;
                while (m_dynamicTableSize > m_currentMaxDynamicTableSize && !m_dynamicTable.empty()) {
                    auto it = m_dynamicTable.back();
                    m_dynamicTableSize -= it.key.length() + it.value.length() + 32;
                    m_dynamicTable.pop_back();
                }

                it += len;
                continue;
            } else {
                // 6.2.2 Literal Header Field without Indexing
                // 6.2.3 Literal Header Field Never Indexed
                len = decode_int(intValue, it, 4);
            }
            it += len;

            if (intValue > 61) {
                return ErrorCompressionError;
            }

            QString key;
            if (intValue != 0) {
                auto h = hpackStaticHeaders[intValue];
                key = h.key;
            } else {
                bool errorUpper = false;
                len = parse_string_key(m_huffmanTree, key, it, itEnd, errorUpper);
                if (errorUpper) {
                    return ErrorProtocolError;
                }
                it += len;
            }

            QString value;
            bool error = false;
            len = parse_string(m_huffmanTree, value, it, itEnd, error);
            if (error) {
                qDebug() << "=========================parsing string error";
                return ErrorCompressionError;
            }
            it += len;

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
                int size = key.length() + value.length() + 32;
                qDebug() << "=========================Adding to dynamic table key/value" << key << value << size;
                while (size + m_dynamicTableSize > m_currentMaxDynamicTableSize && !m_dynamicTable.empty()) {
                    auto it = m_dynamicTable.back();
                    m_dynamicTableSize -= it.key.length() + it.value.length() + 32;
                    m_dynamicTable.pop_back();
                }

                if (size + m_dynamicTableSize > m_currentMaxDynamicTableSize) {
                    m_dynamicTable.insert(m_dynamicTable.begin(), { key, value });
                    m_dynamicTableSize += size;
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

namespace CWSGI {
class Node
{
public:
    Node(qint16 c = -1) : code(c) {}
    ~Node() {
        delete right;
        delete left;
    }

    Node *left = nullptr;
    Node *right = nullptr;
    qint16 code;
};

}
HuffmanTree::HuffmanTree()
    : m_root(new Node(0xffff))
{
    for (qint16 code = 0; code < 257; code++) {
        Node *cursor = m_root;
        huffman_code huff = huffmanTable[code];
        for (int i = huff.bitLen; i > 0; i--) {
            if (huff.code & (1 << (i - 1))) {
                if (!cursor->right) {
                    cursor->right = new Node;
                }
                cursor = cursor->right;
            } else {
                if (!cursor->left) {
                    cursor->left = new Node;
                }
                cursor = cursor->left;
            }
        }
        cursor->code = code;
    }
}

HuffmanTree::~HuffmanTree()
{
    delete m_root;
}

qint64 HuffmanTree::encode(quint8 *buf, const QByteArray &content)
{
    quint8 tmp = 0;
    quint8 bufRest = 8;
    qint64 len = 0;
    for (int i = 0; i < content.length(); i++) {
        huffman_code huff = huffmanTable[quint8(content[i])];
        while (huff.bitLen > 0) {
            if (huff.bitLen < bufRest) {
                bufRest -= huff.bitLen;
                tmp |= quint8(huff.code << bufRest);
                huff.bitLen = 0;
            } else {
                quint8 shift = huff.bitLen-bufRest;
                tmp |= quint8(huff.code >> shift);
                huff.bitLen -= bufRest;
                bufRest = 0;
                huff.code = huff.code & ((1 << shift) - 1);
            }

            if (bufRest == 0) {
                *(buf + (len++)) = tmp;
                bufRest = 8;
                tmp = 0;
            }
        }
    }

    if (bufRest > 0 && bufRest < 8) {
        tmp |= ((1 << bufRest) - 1);
        *(buf + (len++)) = tmp;
    }

    return len;
}

QString HuffmanTree::decode(const quint8 *buf, quint32 str_len, bool &error)
{
    QString dst;
    Node *cursor = m_root;
//    qDebug() << "HuffmanTree::decode" << str_len << QByteArray(reinterpret_cast<const char *>(buf), str_len).toHex();
    for (quint16 i = 0; i < str_len; i++) {
//        qDebug() << "i" << i;
        // THESE two are things I really not sure that are correct
        quint8 pading = 0;
//        quint8 zeroPading = 0;
        for (qint8 j = 7; j >= 0; j--) {
//            if (!*(buf + i)) {
//                zeroPading++;
//            } else {
//                zeroPading = 0;
//            }

            if (*(buf + i) & (1 << j)) {
                cursor = cursor->right;
            } else {
                cursor = cursor->left;
            }

//            qDebug() << "HuffmanTree::decode" << j << cursor->code;
            if (cursor->code >= 0) {
                dst += QLatin1Char(cursor->code);
                cursor = m_root;
            } else {
                ++pading;
            }

            if (pading > 7 /*|| zeroPading > 1*/) {
                qDebug() << "HuffmanTree::decode padding error" << pading;
                error = true;
                return dst;
            }
        }
    }
    qDebug() << "HuffmanTree::decode" << str_len;
    return dst;
}
