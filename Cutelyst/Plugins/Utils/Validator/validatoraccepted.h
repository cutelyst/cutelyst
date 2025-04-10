/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORACCEPTED_H
#define CUTELYSTVALIDATORACCEPTED_H

#include "validatorrule.h"

namespace Cutelyst {

class ValidatorAcceptedPrivate;

/**
 * \ingroup plugins-utils-validator-rules
 * \headerfile "" <Cutelyst/Plugins/Utils/validatoraccepted.h>
 * \brief Checks if a field is available and has a specific value.
 *
 * The \a field under validation must be \c yes, \c on, \c 1, or \c true. This is useful for
 * validating "Terms of Service" acceptance. This check will also fail if the input data for the
 * specified \a field is empty or if the \a field is not part of the input data.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link
 * Validator::NoTrimming NoTrimming\endlink, whitespaces will be removed from the beginning and the
 * end of the input value before validation.
 *
 * \par Return type
 * On success, ValidatorReturnType::value will be set to \c true.
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorAccepted : public ValidatorRule
{
public:
    /**
     * Constructs a new %ValidatorAccepted object for the given \a field using optional
     * custom error \a messages.
     */
    explicit ValidatorAccepted(const QString &field, const ValidatorMessages &messages = {});

    /**
     * Destroys the %ValidatorAccepted object.
     */
    ~ValidatorAccepted() override;

    /**
     * \ingroup plugins-utils-validator-rules
     * \brief Returns \c true if the \a value is equal to \c yes, \c on, \c 1, or \c true.
     *
     * Returns \c true if the \a value is equal to \c yes, \c on, \c 1, or \c true, otherwise
     * returns \c false.
     */
    static bool validate(const QString &value);

protected:
    /**
     * Performs the validation on the input \a params and returns the result.
     *
     * If validation succeeds, ValidatorReturnType::value will contain \c true,
     * otherwise \c false.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /**
     * Performs the validation on the input \a params and writes the result to the \a cb.
     *
     * If validation succeeds, ValidatorReturnType::value will contain \c true,
     * otherwise \c false.
     */
    void validateCb(Context *c, const ParamsMultiMap &params, ValidatorRtFn cb) const override;

    /**
     * Returns a generic error message.
     */
    QString genericValidationError(Context *c,
                                   const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorAccepted) // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)
    Q_DISABLE_COPY(ValidatorAccepted)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORACCEPTED_H
