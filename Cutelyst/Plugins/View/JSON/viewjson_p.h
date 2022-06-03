/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWJSON_P_H
#define VIEWJSON_P_H

#include "viewjson.h"
#include "view_p.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QStringList>
#include <QtCore/QRegularExpression>

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
    bool xJsonHeader = false;
};

}

#endif // VIEWJSON_P_H

