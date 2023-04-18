/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYST_ACTION_P_H
#define CUTELYST_ACTION_P_H

#include "action.h"
#include "component_p.h"

namespace Cutelyst {

class ActionPrivate : public ComponentPrivate
{
public:
    virtual ~ActionPrivate() override = default;

    QString ns;
    QMetaMethod method;
    QMultiMap<QString, QString> attributes;
    Controller *controller = nullptr;
    QStringList emptyArgs  = {
         QString(),
         QString(),
         QString(),
         QString(),
         QString(),
         QString(),
         QString(),
         QString(),
         QString(),
    };
    qint8 numberOfArgs     = -1;
    qint8 numberOfCaptures = -1;
    bool evaluateBool      = false;
    bool listSignature     = false;
};

} // namespace Cutelyst

#endif // CUTELYST_ACTION_P_H
