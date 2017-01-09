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
#ifndef CUTELYSTVALIDATOR_P_H
#define CUTELYSTVALIDATOR_P_H

#include "validator.h"
#include "context.h"
#include "request.h"
#include "validatorrule.h"

namespace Cutelyst {

class ValidatorPrivate
{
public:
    ValidatorPrivate() :
        stopOnFirstError(false)
    {}

    ValidatorPrivate(const ParamsMultiMap &parameters) :
        params(parameters),
        stopOnFirstError(false),
        context(nullptr)
    {}

    ValidatorPrivate(Context *c) :
        stopOnFirstError(false),
        context(c)
    {
        params = c->request()->parameters();
    }

    ParamsMultiMap params;
    bool stopOnFirstError;
    QList<ValidatorRule*> validators;
    QHash<QString,QString> labelDict;
    QString tmpl;
    Context *context;


    void setStashOnInvalid()
    {
        if (!tmpl.isEmpty() && (context != nullptr) && !validators.isEmpty()) {

            QVariantList validationErrorStrings;
            QVariantList validationErrorFields;

            for (int i = 0; i < validators.count(); ++i) {
                ValidatorRule *v = validators.at(i);
                if (!v->isValid()) {
                    validationErrorStrings << v->errorMessage();
                    validationErrorFields << v->field();
                }
            }

            context->setStash(QStringLiteral("validationErrorStrings"), validationErrorStrings);
            context->setStash(QStringLiteral("validationErrorFields"), validationErrorFields);

            if (!params.isEmpty()) {
                QMap<QString,QString>::const_iterator i = params.constBegin();
                while (i != params.constEnd()) {
                    if (!i.key().contains(QStringLiteral("password"), Qt::CaseInsensitive)) {
                        context->setStash(i.key(), i.value());
                    }
                    ++i;
                }
            }

            context->setStash(QStringLiteral("template"), tmpl);
        }
    }
};

}

#endif //CUTELYSTVALIDATOR_P_H

