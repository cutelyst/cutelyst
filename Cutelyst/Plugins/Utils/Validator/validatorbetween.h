/*
 * SPDX-FileCopyrightText: (C) 2017-2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORBETWEEN_H
#define CUTELYSTVALIDATORBETWEEN_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorBetweenPrivate;
/**
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorBetween validatorbetween.h <Cutelyst/Plugins/Utils/validatorbetween.h>
 * \brief Checks if a value or text length is between a minimum and maximum value.
 *
 * This validator has the mandatory extra parameters \a type, \a min and \a max. \a type specifies
 * the type to validate against, it can be either a QMetaType::Type for a number value (Int, UInt,
 * Float, etc.) or QString, that are used as base types. You can compare against any integer,
 * floating point or string type. If you validate a string, it’s length will be checked if it is
 * between \a min and \a max values.
 *
 * If you set a string to the \a min and/or \a max values, this will neither be interpreted as a
 * number nor as string length, but will be used to get the comparison number value from the \link
 * Context::stash() stash\endlink.
 *
 * \note Conversion of numeric input values is performed in the \c 'C' locale.
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
 * Validator v({new ValidatorBetween(QStringLiteral("username"), QMetaType::QString, 3, 255))});
 * \endcode
 *
 * \par Return type
 * On success, ValidatorReturnType::value will contain a value of the \a type specified in the
 * constructor.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorMin, ValidatorMax, ValidatorSize
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorBetween : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorBetween object with the given parameters.
     *
     * \param field      Name of the input field to validate.
     * \param type       The type to compare. Can be either a QMetaType::Type for a number value
     *                   or QMetaType::QString.
     * \param min        Minimum value. Will be converted into comparable value. If it is a
     *                   QString, it will try to get the comparison value from another params
     *                   field or the stash.
     * \param max        Maximum value. Will be converted into comparable value. If it is a
     *                   QString, it will try to get the comparison value from another params
     *                   field or the stash.
     * \param messages   Custom error message if validation fails.
     * \param defValKey  \link Context::stash() Stash \endlink key containing a default value if
     *                   input field is empty. This value will \b NOT be validated.
     */
    ValidatorBetween(const QString &field,
                     QMetaType::Type type,
                     const QVariant &min,
                     const QVariant &max,
                     const ValidatorMessages &messages = ValidatorMessages(),
                     const QString &defValKey          = {});

    /**
     * Destroys the %ValidatorBetween object.
     */
    ~ValidatorBetween() override;

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value
     * converted into the \a type specified in the constructor.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and calls the \a cb with the
     * ValidatorReturnType as argument.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input parameter value
     * converted into the \a type specified in the constructor.
     *
     * \since Cutelyst 5.0.0
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * Returns a generic error message.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

    /**
     * Returns a generic error message for validation data errors.
     */
    QString genericValidationDataError(Context *c, const QVariant &errorData) const override;

    /**
     * Returns a generic error message for input value parsing errors.
     */
    QString genericParsingError(Context *c, const QVariant &errorData) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorBetween) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorBetween)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORBETWEEN_H
