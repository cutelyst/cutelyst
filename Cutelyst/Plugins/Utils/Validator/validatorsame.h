/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORSAME_H
#define CUTELYSTVALIDATORSAME_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorSamePrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatorsame.h>
 * \brief The given field must match the field under validation.
 *
 * The \a field under validation must have the same content as \a otherField.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation. If the \a field's value is empty or if the \a field is
 * missing in the input data, the validation will succeed without performing the validation itself.
 * Use one of the \link ValidatorRequired required validators \endlink to require the field to be
 * present and not empty.
 *
 * \par Example
 * \code{.cpp}
 * void MyController::do_form(Context *c)
 * {
 *     Validator v({new ValidatorSame(QStringLiteral("field"),
 *                                    QStringLiteral("other_field"),
 *                                    QT_TRANSLATE_NOOP("MyController", "Other Field"),
 *                                    ValidatorMessages(QT_TRANSLATE_NOOP("MyController",
 *                                                                        "Field")))
 *                 }, u"MyController"_s);
 * }
 * \endcode
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a QString.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorSame : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorSame object with the given parameters.
     *
     * \param field         Name of the input field to validate.
     * \param otherField    Name of the other field that must have the same input.
     * \param otherLabel    Human readable other field label, used for generic error messages.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value
     *                      if input field is empty. This value will \b NOT be validated.
     */
    ValidatorSame(const QString &field,
                  const QString &otherField,
                  const char *otherLabel            = nullptr,
                  const ValidatorMessages &messages = ValidatorMessages(),
                  const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorSame object.
     */
    ~ValidatorSame() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorSame) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorSame)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORSAME_H
