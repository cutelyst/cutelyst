/*
 * SPDX-FileCopyrightText: (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef HPACKTABLES_H
#define HPACKTABLES_H

#include <Cutelyst/Headers>

#include <QHash>
#include <QString>
#include <QVector>

namespace Cutelyst {

struct DynamicTableEntry {
    QByteArray key;
    QByteArray value;
};

class Headers;
class CWsgiEngine;
class H2Stream;
class HPack
{
public:
    HPack(int maxTableSize);
    ~HPack();

    void encodeHeaders(int status, const Headers &headers, QByteArray &buf, CWsgiEngine *engine);

    int decode(unsigned char *it, unsigned char *itEnd, H2Stream *stream);

private:
    QVector<DynamicTableEntry> m_dynamicTable;
    int m_dynamicTableSize           = 0;
    int m_currentMaxDynamicTableSize = 0;
    int m_maxTableSize;
};

} // namespace Cutelyst

#endif // HPACKTABLES_H
