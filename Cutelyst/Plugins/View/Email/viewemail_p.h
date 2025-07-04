/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWJSON_P_H
#define VIEWJSON_P_H

#include "view_p.h"
#include "viewemail.h"

#include <SimpleMail/mimepart.h>
#include <SimpleMail/server.h>
#include <memory>

#include <QtCore/QStringList>

using namespace SimpleMail;
using namespace Qt::StringLiterals;

namespace Cutelyst {

class ViewEmailPrivate : public ViewPrivate
{
public:
    void setupAttributes(std::shared_ptr<MimePart> part, const QVariantHash &attrs) const;
    void setupEncoding(std::shared_ptr<MimePart> part, const QByteArray &encoding) const;

    QString stashKey              = u"email"_s;
    QByteArray defaultContentType = QByteArrayLiteral("text/plain");
    QByteArray defaultCharset;
    QByteArray defaultEncoding;
    QStringList exposeKeys;
    Server *server = nullptr;
};

} // namespace Cutelyst

#endif // VIEWJSON_P_H
