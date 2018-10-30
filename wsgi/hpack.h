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
#include <QVector>
#include <QHash>

namespace Cutelyst {
class Headers;
}

namespace CWSGI {

struct DynamicTableEntry
{
    QString key;
    QString value;
};

class CWsgiEngine;
class H2Stream;
class HPack
{
public:
    HPack(int maxTableSize);
    ~HPack();

    void encodeHeaders(int status, const QHash<QString, QString> &headers, QByteArray &buf, CWSGI::CWsgiEngine *engine);

    int decode(unsigned char *it, unsigned char *itEnd, H2Stream *stream);

private:
    QVector<DynamicTableEntry> m_dynamicTable;
    int m_dynamicTableSize = 0;
    int m_currentMaxDynamicTableSize = 0;
    int m_maxTableSize;
};

}

#endif // HPACKTABLES_H
