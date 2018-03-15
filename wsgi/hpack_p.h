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
#ifndef HPACK_P_H
#define HPACK_P_H

#include <QString>
#include <QHash>

class HPackPrivate {
public:
    /**
     * huffman decoding state, which is actually the node ID of internal huffman tree.
     * We have 257 leaf nodes, but they are identical to root node other than emitting
     * a symbol, so we have 256 internal nodes [1..255], inclusive
     */

    struct HuffDecode {
       quint8 state,
              flags, // bitwise OR of zero or more of the HuffDecodeFlag
              sym;   // symbol if HUFF_SYM flag set
    };

    struct HuffSym {
       quint32 nbits, // The number of bits in this code
               code;  // Huffman code aligned to LSB
    };

    enum HuffDecodeFlag {
       HUFF_ACCEPTED =  1,       // FSA accepts this state as the end of huffman encoding sequence
       HUFF_SYM      = (1 << 1), // This state emits symbol
       HUFF_FAIL     = (1 << 2)  // If state machine reaches this state, decoding fails
    };

    static const HuffSym huff_sym_table[];
    static const HuffDecode huff_decode_table[][16];
};

typedef struct {
    QString key;
    QString value;
} hpackStaticPair;

static const hpackStaticPair hpackStaticHeaders[] = {

    {QString(), QString()},
    {QStringLiteral(":authority"), QString()},
    {QStringLiteral(":method"), QStringLiteral("GET")},
    {QStringLiteral(":method"), QStringLiteral("POST")},
    {QStringLiteral(":path"), QStringLiteral("/")},
    {QStringLiteral(":path"), QStringLiteral("/index.html")},
    {QStringLiteral(":scheme"), QStringLiteral("http")},
    {QStringLiteral(":scheme"), QStringLiteral("https")},
    {QStringLiteral(":status"), QStringLiteral("200")},
    {QStringLiteral(":status"), QStringLiteral("204")},
    {QStringLiteral(":status"), QStringLiteral("206")},
    {QStringLiteral(":status"), QStringLiteral("304")},
    {QStringLiteral(":status"), QStringLiteral("400")},
    {QStringLiteral(":status"), QStringLiteral("404")},
    {QStringLiteral(":status"), QStringLiteral("500")},
    {QStringLiteral("accept-charset"), QString()},
    {QStringLiteral("accept-encoding"), QStringLiteral("gzip, deflate")},
    {QStringLiteral("accept-language"), QString()},
    {QStringLiteral("accept-ranges"), QString()},
    {QStringLiteral("accept"), QString()},
    {QStringLiteral("access-control-allow-origin"), QString()},
    {QStringLiteral("age"), QString()},
    {QStringLiteral("allow"), QString()},
    {QStringLiteral("authorization"), QString()},
    {QStringLiteral("cache-control"), QString()},
    {QStringLiteral("content-disposition"), QString()},
    {QStringLiteral("content-encoding"), QString()},
    {QStringLiteral("content-language"), QString()},
    {QStringLiteral("content-length"), QString()},
    {QStringLiteral("content-location"), QString()},
    {QStringLiteral("content-range"), QString()},
    {QStringLiteral("content-type"), QString()},
    {QStringLiteral("cookie"), QString()},
    {QStringLiteral("date"), QString()},
    {QStringLiteral("etag"), QString()},
    {QStringLiteral("expect"), QString()},
    {QStringLiteral("expires"), QString()},
    {QStringLiteral("from"), QString()},
    {QStringLiteral("host"), QString()},
    {QStringLiteral("if-match"), QString()},
    {QStringLiteral("if-modified-since"), QString()},
    {QStringLiteral("if-none-match"), QString()},
    {QStringLiteral("if-range"), QString()},
    {QStringLiteral("if-unmodified-since"), QString()},
    {QStringLiteral("last-modified"), QString()},
    {QStringLiteral("link"), QString()},
    {QStringLiteral("location"), QString()},
    {QStringLiteral("max-forwards"), QString()},
    {QStringLiteral("proxy-authenticate"), QString()},
    {QStringLiteral("proxy-authorization"), QString()},
    {QStringLiteral("range"), QString()},
    {QStringLiteral("referer"), QString()},
    {QStringLiteral("refresh"), QString()},
    {QStringLiteral("retry-after"), QString()},
    {QStringLiteral("server"), QString()},
    {QStringLiteral("set-cookie"), QString()},
    {QStringLiteral("strict-transport-security"), QString()},
    {QStringLiteral("transfer-encoding"), QString()},
    {QStringLiteral("user-agent"), QString()},
    {QStringLiteral("vary"), QString()},
    {QStringLiteral("via"), QString()},
    {QStringLiteral("www-authenticate"), QString()}
};






static const QHash<QString, quint16> hpackStaticHeadersCode {
    {QStringLiteral("ACCEPT_CHARSET"), 0x0f00},
//    {QStringLiteral("ACCEPT_ENCODING"), QStringLiteral("gzip, deflate")},
    {QStringLiteral("ACCEPT_LANGUAGE"), 0x0f02},
    {QStringLiteral("ACCEPT_RANGES"), 0x0f03},
    {QStringLiteral("ACCEPT"), 0x0f04},
    {QStringLiteral("ACCESS_CONTROL_ALLOW_ORIGIN"), 0x0f05},
    {QStringLiteral("AGE"), 0x0f06},
    {QStringLiteral("ALLOW"), 0x0f07},
    {QStringLiteral("AUTHORIZATION"), 0x0f08},
    {QStringLiteral("CACHE_CONTROL"), 0x0f09},
    {QStringLiteral("CONTENT_DISPOSITION"), 0x0f0a},
    {QStringLiteral("CONTENT_ENCODING"), 0x0f0b},
    {QStringLiteral("CONTENT_LANGUAGE"), 0x0f0c},
    {QStringLiteral("CONTENT_LENGTH"), 0x0f0d},
    {QStringLiteral("CONTENT_LOCATION"), 0x0f0e},
    {QStringLiteral("CONTENT_RANGE"), 0x0f0f},
    {QStringLiteral("CONTENT_TYPE"), 0x0f10},
    {QStringLiteral("COOKIE"), 0x0f11},
    {QStringLiteral("DATE"), 0x0f12},
    {QStringLiteral("ETAG"), 0x0f13},
    {QStringLiteral("EXPECT"), 0x0f14},
    {QStringLiteral("EXPIRES"), 0x0f15},
    {QStringLiteral("FROM"), 0x0f16},
    {QStringLiteral("HOST"), 0x0f17},
    {QStringLiteral("IF_MATCH"), 0x0f18},
    {QStringLiteral("IF_MODIFIED_SINCE"), 0x0f19},
    {QStringLiteral("IF_NONE_MATCH"), 0x0f1a},
    {QStringLiteral("IF_RANGE"), 0x0f1b},
    {QStringLiteral("IF_UNMODIFIED_SINCE"), 0x0f1c},
    {QStringLiteral("LAST_MODIFIED"), 0x0f1d},
    {QStringLiteral("LINK"), 0x0f1f},
    {QStringLiteral("LOCATION"), 0x0f20},
    {QStringLiteral("MAX_FORWARDS"), 0x0f21},
    {QStringLiteral("PROXY_AUTHENTICATE"), 0x0f22},
    {QStringLiteral("PROXY_AUTHORIZATION"), 0x0f23},
    {QStringLiteral("RANGE"), 0x0f24},
    {QStringLiteral("REFERER"), 0x0f25},
    {QStringLiteral("REFRESH"), 0x0f26},
    {QStringLiteral("RETRY_AFTER"), 0x0f27},
    {QStringLiteral("SERVER"), 0x0f28},
    {QStringLiteral("SET_COOKIE"), 0x0f29},
    {QStringLiteral("STRICT_TRANSPORT_SECURITY"), 0x0f2a},
    {QStringLiteral("TRANSFER_ENCODING"), 0x0f2b},
    {QStringLiteral("USER_AGENT"), 0x0f2c},
    {QStringLiteral("VARY"), 0x0f2d},
    {QStringLiteral("VIA"), 0x0f2e},
    {QStringLiteral("WWW_AUTHENTICATE"), 0x0f2f}
};

#endif // HPACK_P_H
