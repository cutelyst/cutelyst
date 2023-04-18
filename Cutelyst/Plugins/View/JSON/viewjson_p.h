/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWJSON_P_H
#define VIEWJSON_P_H

#include "view_p.h"
#include "viewjson.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QRegularExpression>
#include <QtCore/QStringList>

namespace Cutelyst {

class ViewJsonPrivate : public ViewPrivate
{
public:
    virtual ~ViewJsonPrivate() override = default;

    ViewJson::ExposeMode exposeMode = ViewJson::All;
    QString exposeKey;
    QStringList exposeKeys;
    QRegularExpression exposeRE;
    QJsonDocument::JsonFormat format = QJsonDocument::Compact;
    bool xJsonHeader                 = false;
};

} // namespace Cutelyst

#endif // VIEWJSON_P_H
