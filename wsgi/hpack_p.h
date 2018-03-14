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

#endif // HPACK_P_H
