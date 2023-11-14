/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorboolean_p.h"

#include <QStringList>

using namespace Cutelyst;

const QStringList ValidatorBooleanPrivate::trueVals{u"1"_qs, u"true"_qs, u"on"_qs};
const QStringList ValidatorBooleanPrivate::falseVals{u"0"_qs, u"false"_qs, u"off"_qs};

ValidatorBoolean::ValidatorBoolean(const QString &field,
                                   const ValidatorMessages &messages,
                                   const QString &defValKey)
    : ValidatorRule(*new ValidatorBooleanPrivate(field, messages, defValKey))
{
}

ValidatorBoolean::~ValidatorBoolean() = default;

ValidatorReturnType ValidatorBoolean::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        if (ValidatorBooleanPrivate::trueVals.contains(v, Qt::CaseInsensitive)) {
            result.value.setValue<bool>(true);
        } else if (ValidatorBooleanPrivate::falseVals.contains(v, Qt::CaseInsensitive)) {
            result.value.setValue<bool>(false);
        } else {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " \"" << v << "\" can not be interpreted as boolean";
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorBoolean::genericValidationError(Cutelyst::Context *c,
                                                 const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "Can not be interpreted as boolean value."
        return c->qtTrId("cutelyst-vaboolean-genvalerr");
    } else {
        //: %1 will be replaced by the field label
        //% "The value in the “%1” field can not be interpreted as a boolean value."
        return c->qtTrId("cutelyst-vaboolean-genvalerr-label").arg(_label);
    }
}
