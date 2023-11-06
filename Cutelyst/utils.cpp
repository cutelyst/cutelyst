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
    QTextStream out(&buffer, QTextStream::WriteOnly);
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

QByteArray Utils::buildTable(const QVector<QStringList> &table,
                             const QStringList &headers,
                             const QString &title)
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
    QTextStream out(&buffer, QTextStream::WriteOnly);

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

    char *data           = ba.data();
    const char *inputPtr = data;

    const int len = ba.count();
    bool skipUtf8 = true;
    int outlen    = 0;
    for (int i = 0; i < len; ++i, ++outlen) {
        const char c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            int a = inputPtr[++i];
            int b = inputPtr[++i];

            if (a >= '0' && a <= '9')
                a -= '0';
            else if (a >= 'a' && a <= 'f')
                a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F')
                a = a - 'A' + 10;

            if (b >= '0' && b <= '9')
                b -= '0';
            else if (b >= 'a' && b <= 'f')
                b = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F')
                b = b - 'A' + 10;

            *data++  = (char) ((a << 4) | b);
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

    bool hasKey   = false;
    bool skipUtf8 = true;
    char *from    = data;
    int outlen    = 0;

    auto processKeyPair = [&] {
        if (hasKey) {
            if ((data - from) == 0) {
                if (!key.isEmpty()) {
                    ret.insert(key, {});
                }
            } else {
                ret.insert(key,
                           skipUtf8 ? QString::fromLatin1(from, data - from)
                                    : QString::fromUtf8(from, data - from));
            }
        } else if ((data - from) > 0) {
            ret.insert(skipUtf8 ? QString::fromLatin1(from, data - from)
                                : QString::fromUtf8(from, data - from),
                       {});
        }
    };

    for (int i = 0; i < len; ++i, ++outlen) {
        const char c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            int a = inputPtr[++i];
            int b = inputPtr[++i];

            if (a >= '0' && a <= '9')
                a -= '0';
            else if (a >= 'a' && a <= 'f')
                a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F')
                a = a - 'A' + 10;

            if (b >= '0' && b <= '9')
                b -= '0';
            else if (b >= 'a' && b <= 'f')
                b = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F')
                b = b - 'A' + 10;

            *data++  = (char) ((a << 4) | b);
            skipUtf8 = false;
        } else if (c == '+') {
            *data++ = ' ';
        } else if (c == '=') {
            key      = skipUtf8 ? QString::fromLatin1(from, data - from)
                                : QString::fromUtf8(from, data - from);
            from     = data;
            hasKey   = true;
            skipUtf8 = true; // reset
        } else if (c == '&') {
            processKeyPair();
            key.clear();
            hasKey   = false;
            from     = data;
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

    char *data           = ba->data();
    const char *inputPtr = data;

    int len       = ba->count();
    bool skipUtf8 = true;
    int outlen    = 0;
    for (int i = 0; i < len; ++i, ++outlen) {
        const char c = inputPtr[i];
        if (c == '%' && i + 2 < len) {
            int a = inputPtr[++i];
            int b = inputPtr[++i];

            if (a >= '0' && a <= '9')
                a -= '0';
            else if (a >= 'a' && a <= 'f')
                a = a - 'a' + 10;
            else if (a >= 'A' && a <= 'F')
                a = a - 'A' + 10;

            if (b >= '0' && b <= '9')
                b -= '0';
            else if (b >= 'a' && b <= 'f')
                b = b - 'a' + 10;
            else if (b >= 'A' && b <= 'F')
                b = b - 'A' + 10;

            *data++  = (char) ((a << 4) | b);
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

std::chrono::microseconds Utils::durationFromString(QStringView str, bool *ok)
{
    QList<std::pair<QString, QString>> parts;
    QString digitPart;
    QString unitPart;
    bool valid = true;
    for (const QChar ch : str) {
        if (ch >= QLatin1Char('0') && ch <= QLatin1Char('9')) {
            // NOLINTNEXTLINE(bugprone-branch-clone)
            if (digitPart.isEmpty() && unitPart.isEmpty()) {
                // we are at the beginning of a new part
                digitPart.append(ch);
            } else if (digitPart.isEmpty() && !unitPart.isEmpty()) {
                // wrong order
                valid = false;
                break;
            } else if (!digitPart.isEmpty() && unitPart.isEmpty()) {
                // we are still in the digit part
                digitPart.append(ch);
            } else if (!digitPart.isEmpty() && !unitPart.isEmpty()) {
                // we start a new part
                parts.emplace_back(digitPart, unitPart);
                digitPart.clear();
                unitPart.clear();
                digitPart.append(ch);
            }
        } else if ((ch >= QLatin1Char('a') && ch <= QLatin1Char('z')) || ch == QLatin1Char('M')) {
            // NOLINTNEXTLINE(bugprone-branch-clone)
            if (digitPart.isEmpty() && unitPart.isEmpty()) {
                // something is wrong with a digitless unit
                valid = false;
                break;
            } else if (digitPart.isEmpty() && !unitPart.isEmpty()) {
                // it should not be possible to be here
                valid = false;
                break;
            } else if (!digitPart.isEmpty() && unitPart.isEmpty()) {
                // we start adding the unit
                unitPart.append(ch);
            } else if (!digitPart.isEmpty() && !unitPart.isEmpty()) {
                // normal operation
                unitPart.append(ch);
            }
        }
    }

    if (!valid) {
        if (ok) {
            *ok = false;
        }
        return std::chrono::microseconds::zero();
    }

    if (!digitPart.isEmpty()) {
        parts.emplace_back(digitPart, unitPart);
    }

    if (parts.empty()) {
        if (ok) {
            *ok = false;
        }
        return std::chrono::microseconds::zero();
    }

    std::chrono::microseconds ms = std::chrono::microseconds::zero();

    for (const std::pair<QString, QString> &p : parts) {
        bool _ok             = false;
        const qulonglong dur = p.first.toULongLong(&_ok);
        if (!_ok) {
            valid = false;
            break;
        }

        if (p.second == QLatin1String("usec") || p.second == QLatin1String("us")) {
            ms += std::chrono::microseconds{dur};
        } else if (p.second == QLatin1String("msec") || p.second == QLatin1String("ms")) {
            ms += std::chrono::milliseconds{dur};
        } else if (p.second == QLatin1String("seconds") || p.second == QLatin1String("second") ||
                   p.second == QLatin1String("sec") || p.second == QLatin1String("s") ||
                   p.second.isEmpty()) {
            ms += std::chrono::seconds{dur};
        } else if (p.second == QLatin1String("minutes") || p.second == QLatin1String("minute") ||
                   p.second == QLatin1String("min") || p.second == QLatin1String("m")) {
            ms += std::chrono::minutes{dur};
        } else if (p.second == QLatin1String("hours") || p.second == QLatin1String("hour") ||
                   p.second == QLatin1String("hr") || p.second == QLatin1String("h")) {
            ms += std::chrono::hours{dur};
#if __cplusplus > 201703L
        } else if (p.second == QLatin1String("days") || p.second == QLatin1String("day") ||
                   p.second == QLatin1String("d")) {
            ms += std::chrono::days{dur};
        } else if (p.second == QLatin1String("weeks") || p.second == QLatin1String("week") ||
                   p.second == QLatin1String("w")) {
            ms += std::chrono::weeks{dur};
        } else if (p.second == QLatin1String("months") || p.second == QLatin1String("month") ||
                   p.second == QLatin1String("M")) {
            ms += std::chrono::months{dur};
        } else if (p.second == QLatin1String("years") || p.second == QLatin1String("year") ||
                   p.second == QLatin1String("y")) {
            ms += std::chrono::years{dur};
        }
#else
        } else if (p.second == QLatin1String("days") || p.second == QLatin1String("day") ||
                   p.second == QLatin1String("d")) {
            ms += std::chrono::duration<qulonglong, std::ratio<86400>>{dur};
        } else if (p.second == QLatin1String("weeks") || p.second == QLatin1String("week") ||
                   p.second == QLatin1String("w")) {
            ms += std::chrono::duration<qulonglong, std::ratio<604800>>{dur};
        } else if (p.second == QLatin1String("months") || p.second == QLatin1String("month") ||
                   p.second == QLatin1String("M")) {
            ms += std::chrono::duration<qulonglong, std::ratio<2629746>>{dur};
        } else if (p.second == QLatin1String("years") || p.second == QLatin1String("year") ||
                   p.second == QLatin1String("y")) {
            ms += std::chrono::duration<qulonglong, std::ratio<31556952>>{dur};
        }
#endif
        else {
            valid = false;
            break;
        }
    }

    if (!valid) {
        if (ok) {
            *ok = false;
        }
        return std::chrono::microseconds::zero();
    }

    if (ok) {
        *ok = true;
    }

    return ms;
}
