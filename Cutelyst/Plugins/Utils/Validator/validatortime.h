/*
 * Copyright (C) 2017-2018 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef CUTELYSTVALIDATORTIME_H
#define CUTELYSTVALIDATORTIME_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorTimePrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \brief Checks if the input data is a valid time.
 *
 * This validator checks if the input \a field can be parsed into a QTime, it will check the parsing ability but will not convert the
 * input data into a QTime. If a custom \a format is given, the validator will at first try to parse the date according to that \a format.
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
 * \sa ValidatorDateTime, ValidatorDate
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorTime : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new time validator.
     * \param field         Name of the input field to validate.
     * \param format        Optional time format for input parsing, can be translatable.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorTime(const QString &field, const char *format = nullptr, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());
    
    /*!
     * \brief Deconstructs time the validator.
     */
    ~ValidatorTime();
    
protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramater value converted into a QTime.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;
    
private:
    Q_DECLARE_PRIVATE(ValidatorTime)
    Q_DISABLE_COPY(ValidatorTime)
};
    
}

#endif //CUTELYSTVALIDATORTIME_H

