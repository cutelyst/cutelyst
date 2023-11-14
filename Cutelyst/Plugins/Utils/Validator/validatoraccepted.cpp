/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatoraccepted_p.h"

#include <QStringList>

using namespace Cutelyst;

const QStringList ValidatorAcceptedPrivate::trueVals{u"yes"_qs, u"on"_qs, u"1"_qs, u"true"_qs};

ValidatorAccepted::ValidatorAccepted(const QString &field,
                                     const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorAcceptedPrivate(field, messages))
{
}

ValidatorAccepted::~ValidatorAccepted() = default;

ValidatorReturnType ValidatorAccepted::validate(Cutelyst::Context *c,
                                                const Cutelyst::ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    if (Q_LIKELY(ValidatorAccepted::validate(value(params)))) {
        result.value.setValue<bool>(true);
    } else {
        result.errorMessage = validationError(c);
        result.value.setValue<bool>(false);
        qCDebug(C_VALIDATOR).noquote() << debugString(c);
    }

    return result;
}

bool ValidatorAccepted::validate(const QString &value)
{
    return ValidatorAcceptedPrivate::trueVals.contains(value, Qt::CaseInsensitive);
}

QString ValidatorAccepted::genericValidationError(Cutelyst::Context *c,
                                                  const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "Has to be accepted."
        return c->qtTrId("cutelyst-valaccepted-genvalerr");
    } else {
        //: %1 will be replaced by the field label
        //% "“%1” has to be accepted."
        return c->qtTrId("cutelyst-valaccepted-genvalerr-label").arg(_label);
    }
}
