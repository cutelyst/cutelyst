/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTMEMCACHEDSESSIONSTORE_P_H
#define CUTELYSTMEMCACHEDSESSIONSTORE_P_H

#include "memcachedsessionstore.h"

namespace Cutelyst {

class MemcachedSessionStorePrivate
{
public:
    QByteArray groupKey;

    static const QString stashKeyMemcdSave;
    static const QString stashKeyMemcdData;
};

} // namespace Cutelyst

#endif // CUTELYSTMEMCACHEDSESSIONSTORE_P_H
