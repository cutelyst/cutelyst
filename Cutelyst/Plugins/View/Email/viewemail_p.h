/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWJSON_P_H
#define VIEWJSON_P_H

#include "view_p.h"
#include "viewemail.h"

#include <SimpleMail/mimepart.h>
#include <SimpleMail/sender.h>
#include <SimpleMail/server.h>

#include <QtCore/QStringList>

using namespace SimpleMail;
namespace Cutelyst {

class ViewEmailPrivate : public ViewPrivate
{
public:
    virtual ~ViewEmailPrivate() override = default;
    void setupAttributes(MimePart *part, const QVariantHash &attrs) const;
    void setupEncoding(MimePart *part, const QByteArray &encoding) const;

    QString stashKey              = QStringLiteral("email");
    QByteArray defaultContentType = QByteArrayLiteral("text/plain");
    QByteArray defaultCharset;
    QByteArray defaultEncoding;
    QStringList exposeKeys;
    Sender *sender;
    Server *server = nullptr;
};

} // namespace Cutelyst

#endif // VIEWJSON_P_H
