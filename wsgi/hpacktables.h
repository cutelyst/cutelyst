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
#ifndef HPACKTABLES_H
#define HPACKTABLES_H

#include <QString>
#include <vector>

#include "socket.h"

namespace CWSGI {

class HPackHeaders
{
public:
    HPackHeaders(int size = 4096);

    bool updateTableSize(uint size);

    void push_back(const std::pair<QString, QString> &pair) {
        headers.push_back(pair);
    }

    std::vector<std::pair<QString, QString>> headers;
};

class HuffmanTree;
class HPackTables
{
public:
    HPackTables();

    void encodeStatus(int status);

    void encodeHeader(const QByteArray &key, const QByteArray &value);

    QByteArray data() const;

    static int decode(const quint8 *it, const quint8 *itEnd, HPackHeaders &headers, HuffmanTree *hTree, Socket::H2Stream *stream);

    static std::pair<QString, QString> header(int index);

private:
    QByteArray buf;
};

class Node;
class HuffmanTree {
public:
    HuffmanTree(int tableSize = 257);
    ~HuffmanTree();
    qint64 encode(quint8 *buf, const QByteArray &content);
    QString decode(const quint8 *buf, quint32 len, bool &error);

private:
    Node *m_root;
};

}

#endif // HPACKTABLES_H
