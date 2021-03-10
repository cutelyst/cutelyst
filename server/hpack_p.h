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

    typedef struct {
        QString key;
        QString value;
    } hpackStaticPair;

    static const QHash<QString, const char *> hpackStaticHeadersCode;
    static const hpackStaticPair hpackStaticHeaders[];

    static const HuffSym huff_sym_table[];
    static const HuffDecode huff_decode_table[][16];
};

#endif // HPACK_P_H
