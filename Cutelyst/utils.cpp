/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "utils.h"

#include <QTextStream>
#include <QVector>

using namespace Cutelyst;

QByteArray buildTableDivision(const QVector<int> &columnsSize)
{
    QByteArray buffer;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QTextStream out(&buffer, QTextStream::WriteOnly);
#else
    QTextStream out(&buffer, QIODevice::WriteOnly);
#endif
    for (int i = 0; i < columnsSize.size(); ++i) {
        if (i) {
            out << '+';
        } else {
            out << '.';
        }
        out << QByteArray().fill('-', columnsSize[i] + 2).data();
    }
    out << '.';

    return buffer;
}

QByteArray Utils::buildTable(const QVector<QStringList> &table, const QStringList &headers, const QString &title)
{
    QByteArray buffer;
    QVector<int> columnsSize;

    if (!headers.isEmpty()) {
        for (const QString &header : headers) {
            columnsSize.push_back(header.size());
        }
    } else {
        for (const QStringList &rows : table) {
            if (columnsSize.empty()) {
                for (const QString &row : rows) {
                    columnsSize.push_back(row.size());
                }
            } else if (rows.size() != columnsSize.size()) {
                qFatal("Incomplete table");
            }
        }
    }

    for (const QStringList &row : table) {
        if (row.size() > columnsSize.size()) {
            qFatal("Incomplete table");
            break;
        }

        for (int i = 0; i < row.size(); ++i) {
            columnsSize[i] = qMax(columnsSize[i], row[i].size());
        }
    }

    // printing
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    QTextStream out(&buffer, QTextStream::WriteOnly);
#else
    QTextStream out(&buffer, QIODevice::WriteOnly);
#endif

    out.setFieldAlignment(QTextStream::AlignLeft);
    QByteArray div = buildTableDivision(columnsSize);

    if (!title.isEmpty()) {
        out << title << '\n';
    }

    // Top line
    out << div << '\n';

    if (!headers.isEmpty()) {
        // header titles
        for (int i = 0; i < headers.size(); ++i) {
            out << "| ";

            out.setFieldWidth(columnsSize[i]);
            out << headers[i];

            out.setFieldWidth(0);
            out << ' ';
        }
        out << '|' << '\n';

        // header bottom line
        out << div << '\n';
    }

    for (const QStringList &row : table) {
        // content table
        for (int i = 0; i < row.size(); ++i) {
            out << "| ";

            out.setFieldWidth(columnsSize[i]);
            out << row[i];

            out.setFieldWidth(0);
            out << ' ';
        }
        out << '|' << '\n';
    }

    // table bottom line
    out << div;

    return buffer;
}

QString Utils::decodePercentEncoding(QString *s)
{
    if (s->isEmpty()) {
        return *s;
    }

    QByteArray ba = s->toLatin1();

    char *data = ba.data();
    const char *inputPtr = data;

    const int len = ba.count();
    bool skipUtf8 = true;
    int outlen = 0;
    for (int i = 0 ; i < len; ++i, ++outlen) {
        const char c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            int a = inputPtr[++i];
            int b = inputPtr[++i];

            if (a >= '0' && a <= '9') a -= '0';
            else if (a >= 'a' && a <= 'f') a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F') a = a - 'A' + 10;

            if (b >= '0' && b <= '9') b -= '0';
            else if (b >= 'a' && b <= 'f') b  = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F') b  = b - 'A' + 10;

            *data++ = (char)((a << 4) | b);
            skipUtf8 = false;
        } else if (c == '+') {
            *data++ = ' ';
        } else {
            *data++ = c;
        }
    }

    if (skipUtf8) {
        return *s;
    }

    return QString::fromUtf8(ba.data(), outlen);
}

ParamsMultiMap Utils::decodePercentEncoding(char *data, int len)
{
    ParamsMultiMap ret;
    if (len <= 0) {
        return ret;
    }

    QString key;

    const char *inputPtr = data;

    bool hasKey = false;
    bool skipUtf8 = true;
    char *from = data;
    int outlen = 0;

    auto processKeyPair = [&] {
        if (hasKey) {
            if ((data - from) == 0) {
                if (!key.isEmpty()) {
                    ret.insertMulti(key, {});
                }
            } else {
                ret.insertMulti(key, skipUtf8 ? QString::fromLatin1(from, data - from) : QString::fromUtf8(from, data - from));
            }
        } else if ((data - from) > 0) {
            ret.insertMulti(skipUtf8 ? QString::fromLatin1(from, data - from) : QString::fromUtf8(from, data - from), {});
        }
    };

    for (int i = 0; i < len; ++i, ++outlen) {
        const char c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            int a = inputPtr[++i];
            int b = inputPtr[++i];

            if (a >= '0' && a <= '9') a -= '0';
            else if (a >= 'a' && a <= 'f') a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F') a = a - 'A' + 10;

            if (b >= '0' && b <= '9') b -= '0';
            else if (b >= 'a' && b <= 'f') b  = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F') b  = b - 'A' + 10;

            *data++ = (char)((a << 4) | b);
            skipUtf8 = false;
        } else if (c == '+') {
            *data++ = ' ';
        } else if (c == '=') {
            key = skipUtf8 ? QString::fromLatin1(from, data - from) : QString::fromUtf8(from, data - from);
            from = data;
            hasKey = true;
            skipUtf8 = true; // reset
        } else if (c == '&') {
            processKeyPair();
            key.clear();
            hasKey = false;
            from = data;
            skipUtf8 = true; // reset
        } else {
            *data++ = c;
        }
    }

    processKeyPair();

    return ret;
}

QString Utils::decodePercentEncoding(QByteArray *ba)
{
    if (ba->isEmpty()) {
        return {};
    }

    char *data = ba->data();
    const char *inputPtr = data;

    int len = ba->count();
    bool skipUtf8 = true;
    int outlen = 0;
    for (int i = 0; i < len; ++i, ++outlen) {
        const char c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            int a = inputPtr[++i];
            int b = inputPtr[++i];

            if (a >= '0' && a <= '9') a -= '0';
            else if (a >= 'a' && a <= 'f') a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F') a = a - 'A' + 10;

            if (b >= '0' && b <= '9') b -= '0';
            else if (b >= 'a' && b <= 'f') b  = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F') b  = b - 'A' + 10;

            *data++ = (char)((a << 4) | b);
            skipUtf8 = false;
        } else if (c == '+') {
            *data++ = ' ';
        } else {
            *data++ = c;
        }
    }

    if (skipUtf8) {
        return QString::fromLatin1(ba->data(), outlen);
    } else {
        return QString::fromUtf8(ba->data(), outlen);
    }
}
