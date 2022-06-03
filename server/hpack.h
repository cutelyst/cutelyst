/*
 * SPDX-FileCopyrightText: (C) 2018 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef HPACKTABLES_H
#define HPACKTABLES_H

#include <QString>
#include <QVector>
#include <QHash>

namespace Cutelyst {

struct DynamicTableEntry
{
    QString key;
    QString value;
};

class Headers;
class CWsgiEngine;
class H2Stream;
class HPack
{
public:
    HPack(int maxTableSize);
    ~HPack();

    void encodeHeaders(int status, const QMultiHash<QString, QString> &headers, QByteArray &buf, CWsgiEngine *engine);

    int decode(unsigned char *it, unsigned char *itEnd, H2Stream *stream);

private:
    QVector<DynamicTableEntry> m_dynamicTable;
    int m_dynamicTableSize = 0;
    int m_currentMaxDynamicTableSize = 0;
    int m_maxTableSize;
};

}

#endif // HPACKTABLES_H
