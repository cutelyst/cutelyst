/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORRESULT_P_H
#define CUTELYSTVALIDATORRESULT_P_H

#include "validatorresult.h"

#include <QSharedData>

namespace Cutelyst {

class ValidatorResultPrivate : public QSharedData
{
public:
    ValidatorResultPrivate() {}

    ValidatorResultPrivate(const ValidatorResultPrivate &other)
        : QSharedData(other)
        , errors(other.errors)
        , values(other.values)
        , extras(other.extras)
    {
    }

    ~ValidatorResultPrivate() {}

    QHash<QString, QStringList> errors;
    QVariantHash values;
    QVariantHash extras;
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORRESULT_P_H
