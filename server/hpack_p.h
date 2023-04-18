/*
 * SPDX-FileCopyrightText: (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef HPACK_P_H
#define HPACK_P_H

#include <QHash>
#include <QString>

class HPackPrivate
{
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
            code;      // Huffman code aligned to LSB
    };

    enum HuffDecodeFlag {
        HUFF_ACCEPTED = 1,        // FSA accepts this state as the end of huffman encoding sequence
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
