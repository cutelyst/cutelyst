/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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

#include "validatorjson_p.h"
#include <QJsonDocument>

using namespace Cutelyst;

ValidatorJson::ValidatorJson(const QString &field, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorJsonPrivate(field, label, customError))
{
}

ValidatorJson::ValidatorJson(ValidatorJsonPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorJson::~ValidatorJson()
{
}

QString ValidatorJson::validate() const
{
    QString result;

    const QString v = value();

    if (!v.isEmpty()) {
        const QJsonDocument json = QJsonDocument::fromJson(v.toUtf8());
        if (json.isEmpty() || json.isNull()) {
            result = validationError();
        }
    }

    return result;
}

QString ValidatorJson::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Invalid JSON data.");
    } else {
        error = QStringLiteral("The data entered in the “%1” field is not valid JSON.").arg(label());
    }
    return error;
}
