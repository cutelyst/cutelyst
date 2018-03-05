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

namespace Cutelyst {
class Headers;
}

namespace CWSGI {

class H2Stream;

class HuffmanTree;
class HPack
{
public:
    HPack(int maxTableSize);
    ~HPack();

    void encodeStatus(int status);

    void encodeHeader(const QByteArray &key, const QByteArray &value);

    QByteArray data() const;

    int decode(const quint8 *it, const quint8 *itEnd, H2Stream *stream);

private:
    std::vector<std::pair<QString, QString>> m_dynamicTable;
    HuffmanTree *m_huffmanTree;
    QByteArray buf;
    quint32 m_maxTableSize;
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
