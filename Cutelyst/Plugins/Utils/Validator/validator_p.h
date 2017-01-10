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
    ValidatorPrivate(Context *c) :
        stopOnFirstError(false),
        context(c)
    {
        if (context) {
            params = c->request()->parameters();
        }
    }

#ifdef Q_COMPILER_INITIALIZER_LISTS
    ValidatorPrivate(Context *c, std::initializer_list<ValidatorRule*> vals) :
        stopOnFirstError(false),
        validators(vals),
        context(c)
    {
        if (context) {
            params = c->request()->parameters();
        }
    }

    ValidatorPrivate(Context *c, std::initializer_list<ValidatorRule*> vals, std::initializer_list<std::pair<QString, QString> > labelDictionary) :
        stopOnFirstError(false),
        validators(vals),
        labelDict(labelDictionary),
        context(c)
    {
        if (context) {
            params = c->request()->parameters();
        }
    }
#endif

    ValidatorPrivate(const ParamsMultiMap &parameters) :
        params(parameters),
        stopOnFirstError(false)
    {}

    ParamsMultiMap params;
    bool stopOnFirstError;
    std::vector<ValidatorRule*> validators;
    QHash<QString,QString> labelDict;
    QString tmpl;
    Context *context;


    void setStashOnInvalid()
    {
        if (!tmpl.isEmpty() && (context != nullptr) && !validators.empty()) {

            QVariantList validationErrorStrings;
            QVariantList validationErrorFields;

            for (size_t i = 0; i < validators.size(); ++i) {
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

