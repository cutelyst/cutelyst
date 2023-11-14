/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatordifferent_p.h"

using namespace Cutelyst;

ValidatorDifferent::ValidatorDifferent(const QString &field,
                                       const QString &other,
                                       const char *otherLabel,
                                       const Cutelyst::ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorDifferentPrivate(field, other, otherLabel, messages))
{
}

ValidatorDifferent::~ValidatorDifferent() = default;

ValidatorReturnType ValidatorDifferent::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorDifferent);

    const QString v = value(params);
    const QString o =
        trimBefore() ? params.value(d->otherField).trimmed() : params.value(d->otherField);

    if (!v.isEmpty()) {
        if ((v == o)) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR).noquote().nospace()
                << debugString(c) << " The value in \"" << d->otherField
                << "\" is not different: \"" << v << "\" == \"" << o << "\"";
        } else {
            result.value.setValue(v);
        }
    }

    return result;
}

QString ValidatorDifferent::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorDifferent);

    Q_UNUSED(errorData);

    const QString _label      = label(c);
    const QString _otherLabel = [d, c]() -> QString {
        if (d->otherLabel) {
            return d->translationContext ? c->translate(d->translationContext, d->otherLabel)
                                         : c->qtTrId(d->otherLabel);
        } else {
            return {};
        }
    }();

    if (_label.isEmpty()) {
        //: %1 will be replaced by the other field’s label or name to compare with
        //% "Has to be different from the value in the “%1” field."
        return c->qtTrId("cutelyst-valdifferent-genvalerr")
            .arg(!_otherLabel.isEmpty() ? _otherLabel : d->otherField);
    } else {
        //: %1 will be replaced by the field label, %2 will be replaced by the other field’s label
        //: to compare with
        //% "The value in the “%1” field has to be different from the value in the “%2“ field."
        return c->qtTrId("cutelyst-valdifferent-genvalerr-label")
            .arg(_label, !_otherLabel.isEmpty() ? _otherLabel : d->otherField);
    }
}
