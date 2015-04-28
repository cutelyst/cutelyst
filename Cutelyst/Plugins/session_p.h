/*
 * Copyright (C) 2014 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef SESSION_P_H
#define SESSION_P_H

#include "session.h"

#include <QRegularExpression>

namespace Cutelyst {

class SessionPrivate : public QObject
{
    Q_OBJECT
    Q_DECLARE_PUBLIC(Session)
public:
    Session *q_ptr;

    static QString filePath(const QString &sessionId);
    QString generateSessionId() const;
    QString getSessionId(Context *c, bool create) const;
    void saveSession(Context *c);
    QVariant loadSession(Context *c);

    QString sessionName;
    QRegularExpression removeRE = QRegularExpression(QStringLiteral("-|{|}"));
};

}

#endif // SESSION_P_H
