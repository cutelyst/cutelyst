/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef VIEWJSON_P_H
#define VIEWJSON_P_H

#include "viewemail.h"
#include "component_p.h"

#include <QtCore/QStringList>

#include <SimpleMail/sender.h>

using namespace SimpleMail;
namespace Cutelyst {

class ViewEmailPrivate : public ComponentPrivate
{
public:
    void setupAttributes(MimePart *part, const QVariantHash &attrs) const;
    void setupEncoding(MimePart *part, const QByteArray &encoding) const;

    QString stashKey = QStringLiteral("email");
    QByteArray defaultContentType = QByteArrayLiteral("text/plain");
    QByteArray defaultCharset;
    QByteArray defaultEncoding;
    QStringList exposeKeys;
    Sender *sender;
};

}

#endif // VIEWJSON_P_H

