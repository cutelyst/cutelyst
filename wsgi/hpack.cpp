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

#include "socket.h"

#include <vector>

#include <QDebug>

using namespace CWSGI;

static const std::pair<QString, QString> staticHeaders;

typedef struct {
    quint32 code;
    quint8 bitLen;
} huffman_code;

quint64 decode_int(quint32 &dst, const quint8 *buf, quint8 N)
{
    quint64 len = 1;
    quint16 twoN = (1 << N) -1;
    dst = *buf & twoN;
    if (dst == twoN) {
        int M = 0;
        do {
            dst += (*(buf+len) & 0x7f) << M;
            M += 7;
        }
        while (*(buf+(len++)) & 0x80);
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

quint64 parse_string(HuffmanTree *huffman, QString &dst, const quint8 *buf, bool &error)
{
    quint32 str_len = 0;
    quint64 len = decode_int(str_len, buf, 7);
    if (*buf & 0x80) {
        qDebug() << "HUFFMAN value" << len << str_len;
        dst = huffman->decode(buf + len, str_len, error);
    } else {
        for (uint i = 0; i < str_len; i++) {
            dst += QLatin1Char(*(buf + (len + i)));
        }
    }
    return len + str_len;
}

quint64 parse_string_key(HuffmanTree *huffman, QString &dst, const quint8 *buf, bool &error)
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

HPack::HPack(int maxTableSize) : m_maxTableSize(maxTableSize)
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
    qDebug() << k << v << stream->path << stream->method << stream->authority << stream->scheme;
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
        if (stream->authority.isEmpty()) {
            stream->authority = v;
            return true;
        }
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
        quint8 kind = *it & 0xF0;
        if (kind & 0x20) {
            quint32 size(0);
            quint64 len = decode_int(size, it, 5);
            qDebug() << "6.3 Dynamic Table update" << *it << "size" << size << len << "allowedToUpdate" << allowedToUpdate;
            if (size > m_maxTableSize) {
                qDebug() << "Trying to update beyond limits";
                return ErrorCompressionError;
            }
            if (!allowedToUpdate) {
//                return ErrorCompressionError;
            }
            m_dynamicTable.reserve(size);

            it += len;
        } else if (kind & 0x80){
            quint32 index(0);
            quint64 len = decode_int(index, it, 7);
            qDebug() << "6.1 Indexed Header Field Representation" << *it << index << len;
            if (index == 0) {
                return ErrorCompressionError;
            }

            QString key;
            QString value;
            if (index > 61) {
                qDebug() << "6.1 Indexed Header Field Representation dynamic table lookup" << *it << index << len;
                index -= 61;
                if (index < m_dynamicTable.size()) {
                    auto h = m_dynamicTable.at(index);
                    key = h.first;
                    value = h.second;
                    qDebug() << "=========================GETTING from dynamic table key/value" << key << value;
                } else {
                    return ErrorCompressionError;
                }
            } else  {
                auto h = hpackStaticHeaders[index];
                key = h.first;
                value = h.second;
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
            qDebug() << "else" << *it;

            quint32 index(0);
            QString key;
            quint64 len = 0;
            if (kind == 0x40) {
                // 6.2.1 Literal Header Field with Incremental Indexing
                len = decode_int(index, it, 6);
            } else if (kind == 0x00) {
                // 6.2.2 Literal Header Field without Indexing
                len = decode_int(index, it, 4);
            } else {
                // 6.2.2 Literal Header Field Never Inded
                len = decode_int(index, it, 4);
            }
            it += len;

            if (index > 61) {
                return ErrorCompressionError;
            }

            if (index != 0) {
                qDebug() << "header index" << index;

                auto h = hpackStaticHeaders[index];
                qDebug() << "header key" << h.first << h.second;

                key = h.first;
            } else {
                qDebug() << "header parse key";
                bool errorUpper = false;
                len = parse_string_key(m_huffmanTree, key, it, errorUpper);
                if (errorUpper) {
                    return ErrorProtocolError;
                }
                it += len;
            }

            QString value;
            bool error = false;
            len = parse_string(m_huffmanTree, value, it, error);
            if (error) {
                return ErrorCompressionError;
            }
            it += len;


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

            if (kind == 0x40) {
                qDebug() << "=========================Adding to dynamic table key/value" << key << value;
                m_dynamicTable.push_back({ key, value });
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

static const huffman_code HUFFMAN_TABLE[] = {

    {0x1ff8, 13},
    {0x7fffd8, 23},
    {0xfffffe2, 28},
    {0xfffffe3, 28},
    {0xfffffe4, 28},
    {0xfffffe5, 28},
    {0xfffffe6, 28},
    {0xfffffe7, 28},
    {0xfffffe8, 28},
    {0xffffea, 24},
    {0x3ffffffc, 30},
    {0xfffffe9, 28},
    {0xfffffea, 28},
    {0x3ffffffd, 30},
    {0xfffffeb, 28},
    {0xfffffec, 28},
    {0xfffffed, 28},
    {0xfffffee, 28},
    {0xfffffef, 28},
    {0xffffff0, 28},
    {0xffffff1, 28},
    {0xffffff2, 28},
    {0x3ffffffe, 30},
    {0xffffff3, 28},
    {0xffffff4, 28},
    {0xffffff5, 28},
    {0xffffff6, 28},
    {0xffffff7, 28},
    {0xffffff8, 28},
    {0xffffff9, 28},
    {0xffffffa, 28},
    {0xffffffb, 28},
    {0x14, 6},
    {0x3f8, 10},
    {0x3f9, 10},
    {0xffa, 12},
    {0x1ff9, 13},
    {0x15, 6},
    {0xf8, 8},
    {0x7fa, 11},
    {0x3fa, 10},
    {0x3fb, 10},
    {0xf9, 8},
    {0x7fb, 11},
    {0xfa, 8},
    {0x16, 6},
    {0x17, 6},
    {0x18, 6},
    {0x0, 5},
    {0x1, 5},
    {0x2, 5},
    {0x19, 6},
    {0x1a, 6},
    {0x1b, 6},
    {0x1c, 6},
    {0x1d, 6},
    {0x1e, 6},
    {0x1f, 6},
    {0x5c, 7},
    {0xfb, 8},
    {0x7ffc, 15},
    {0x20, 6},
    {0xffb, 12},
    {0x3fc, 10},
    {0x1ffa, 13},
    {0x21, 6},
    {0x5d, 7},
    {0x5e, 7},
    {0x5f, 7},
    {0x60, 7},
    {0x61, 7},
    {0x62, 7},
    {0x63, 7},
    {0x64, 7},
    {0x65, 7},
    {0x66, 7},
    {0x67, 7},
    {0x68, 7},
    {0x69, 7},
    {0x6a, 7},
    {0x6b, 7},
    {0x6c, 7},
    {0x6d, 7},
    {0x6e, 7},
    {0x6f, 7},
    {0x70, 7},
    {0x71, 7},
    {0x72, 7},
    {0xfc, 8},
    {0x73, 7},
    {0xfd, 8},
    {0x1ffb, 13},
    {0x7fff0, 19},
    {0x1ffc, 13},
    {0x3ffc, 14},
    {0x22, 6},
    {0x7ffd, 15},
    {0x3, 5},
    {0x23, 6},
    {0x4, 5},
    {0x24, 6},
    {0x5, 5},
    {0x25, 6},
    {0x26, 6},
    {0x27, 6},
    {0x6, 5},
    {0x74, 7},
    {0x75, 7},
    {0x28, 6},
    {0x29, 6},
    {0x2a, 6},
    {0x7, 5},
    {0x2b, 6},
    {0x76, 7},
    {0x2c, 6},
    {0x8, 5},
    {0x9, 5},
    {0x2d, 6},
    {0x77, 7},
    {0x78, 7},
    {0x79, 7},
    {0x7a, 7},
    {0x7b, 7},
    {0x7ffe, 15},
    {0x7fc, 11},
    {0x3ffd, 14},
    {0x1ffd, 13},
    {0xffffffc, 28},
    {0xfffe6, 20},
    {0x3fffd2, 22},
    {0xfffe7, 20},
    {0xfffe8, 20},
    {0x3fffd3, 22},
    {0x3fffd4, 22},
    {0x3fffd5, 22},
    {0x7fffd9, 23},
    {0x3fffd6, 22},
    {0x7fffda, 23},
    {0x7fffdb, 23},
    {0x7fffdc, 23},
    {0x7fffdd, 23},
    {0x7fffde, 23},
    {0xffffeb, 23},
    {0x7fffdf, 23},
    {0xffffec, 24},
    {0xffffed, 24},
    {0x3fffd7, 22},
    {0x7fffe0, 23},
    {0xffffee, 24},
    {0x7fffe1, 23},
    {0x7fffe2, 23},
    {0x7fffe3, 23},
    {0x7fffe4, 23},
    {0x1fffdc, 21},
    {0x3fffd8, 22},
    {0x7fffe5, 23},
    {0x3fffd9, 22},
    {0x7fffe6, 23},
    {0x7fffe7, 23},
    {0xffffef, 24},
    {0x3fffda, 22},
    {0x1fffdd, 21},
    {0xfffe9, 20},
    {0x3fffdb, 22},
    {0x3fffdc, 22},
    {0x7fffe8, 23},
    {0x7fffe9, 23},
    {0x1fffde, 21},
    {0x7fffde, 23},
    {0x3fffdd, 22},
    {0x3fffde, 22},
    {0xfffff0, 24},
    {0x1fffdf, 21},
    {0x3fffdf, 22},
    {0x7fffeb, 23},
    {0x7fffec, 23},
    {0x1fffe0, 21},
    {0x1fffe1, 21},
    {0x3fffe0, 22},
    {0x1fffe2, 21},
    {0x7fffed, 23},
    {0x3fffe1, 22},
    {0x7fffee, 23},
    {0x7fffef, 23},
    {0xfffea, 20},
    {0x3fffe2, 22},
    {0x3fffe3, 22},
    {0x3fffe4, 22},
    {0x7ffff0, 23},
    {0x3fffe5, 22},
    {0x3fffe6, 22},
    {0x7ffff1, 23},
    {0x3ffffe0, 26},
    {0x3ffffe1, 26},
    {0xfffeb, 20},
    {0x7fff1, 19},
    {0x3fffe7, 22},
    {0x7ffff2, 23},
    {0x3fffe8, 22},
    {0x1ffffec, 25},
    {0x3ffffe2, 26},
    {0x3ffffe3, 26},
    {0x3ffffe4, 26},
    {0x7ffffde, 27},
    {0x7ffffdf, 27},
    {0x3ffffe5, 26},
    {0xfffff1, 24},
    {0x1ffffed, 25},
    {0x7fff2, 19},
    {0x1fffe3, 21},
    {0x3ffffe6, 26},
    {0x7ffffe0, 27},
    {0x7ffffe1, 27},
    {0x3ffffe7, 26},
    {0x3ffffe2, 27},
    {0xfffff2, 24},
    {0x1fffe4, 21},
    {0x1fffe5, 21},
    {0x3ffffe8, 26},
    {0x3ffffe9, 26},
    {0xffffffd, 28},
    {0x7ffffe3, 27},
    {0x7ffffe4, 27},
    {0x7ffffe5, 27},
    {0xfffec, 20},
    {0xfffff3, 24},
    {0xfffed, 20},
    {0x1fffe6, 21},
    {0x3fffe9, 22},
    {0x1fffe7, 21},
    {0x1fffe8, 21},
    {0x7ffff3, 23},
    {0x3fffea, 22},
    {0x3fffeb, 22},
    {0x1ffffee, 25},
    {0x1ffffef, 25},
    {0xfffff4, 24},
    {0xfffff5, 24},
    {0x3ffffea, 26},
    {0x7ffff4, 23},
    {0x3ffffeb, 26},
    {0x7ffffe6, 27},
    {0x3ffffec, 26},
    {0x3ffffed, 26},
    {0x7ffffe7, 27},
    {0x7ffffe8, 27},
    {0x7ffffe9, 27},
    {0x7ffffea, 27},
    {0x7ffffeb, 27},
    {0xffffffe, 28},
    {0x7ffffec, 27},
    {0x7ffffed, 27},
    {0x7ffffee, 27},
    {0x7ffffef, 27},
    {0x7fffff0, 27},
    {0x3ffffee, 26},
    {0x3fffffff, 30}
};

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
HuffmanTree::HuffmanTree(int tableSize)
//    : m_root(new Node(0xffffffff))
    : m_root(new Node(0xffff))
{
    for (qint16 code = 0; code < tableSize; code++) {
        Node *cursor = m_root;
        huffman_code huff = HUFFMAN_TABLE[code];
        for (int i = huff.bitLen; i > 0; i--) {
            if (huff.code & (1 << (i-1))) {
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
        huffman_code huff = HUFFMAN_TABLE[quint8(content[i])];
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
        quint8 pading = 0;
        for (qint8 j = 7; j >= 0; j--) {
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

            if (pading > 7) {
                qDebug() << "HuffmanTree::decode padding error" << pading;
                error = true;
                return dst;
            }
        }
    }
    qDebug() << "HuffmanTree::decode" << str_len;
    return dst;
}
