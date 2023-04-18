/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTVALIDATORDATE_H
#define CUTELYSTVALIDATORDATE_H

#include "validatorrule.h"

#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class ValidatorDatePrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorDate validatordate.h <Cutelyst/Plugins/Utils/validatordate.h>
 * \brief Checks if the input data is a valid date.
 *
 * This validator checks if the input \a field can be parsed into a QDate, it will check the parsing ability and will convert the
 * input data into a QDate. If a custom \a inputFormat is given, the validator will at first try to parse the date according to that format.
 * If that fails or if there is no custom \a inputFormat set, it will try to parse the date based on standard formats in the following order:
 * \link Context::locale() Context locale's \endlink \link QLocale::toDate() toDate() \endlink with QLocale::ShortFormat and QLocale::LongFormat,
 * Qt::ISODate, Qt::RFC2822Date, Qt::TextDate
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \sa Validator for general usage of validators.
 *
 * \sa ValidatorDateTime, ValidatorTime
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorDate : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new date validator.
     * \param field         Name of the input field to validate.
     * \param inputFormat   Optional input format for input data parsing, can be translatable.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorDate(const QString &field, const char *inputFormat = nullptr, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());

    /*!
     * \brief Deconstructs the date validator.
     */
    ~ValidatorDate() override;

protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramater value converted into a QDate.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;

private:
    Q_DECLARE_PRIVATE(ValidatorDate)
    Q_DISABLE_COPY(ValidatorDate)
};

} // namespace Cutelyst

#endif // CUTELYSTVALIDATORDATE_H
