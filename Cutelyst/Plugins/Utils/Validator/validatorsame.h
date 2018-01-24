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
#ifndef CUTELYSTVALIDATORSAME_H
#define CUTELYSTVALIDATORSAME_H

#include <Cutelyst/cutelyst_global.h>
#include "validatorrule.h"

namespace Cutelyst {
    
class ValidatorSamePrivate;

/*!
 * \ingroup plugins-utils-validator-rules
 * \class ValidatorSame validatorsame.h <Cutelyst/Plugins/Utils/validatorsame.h>
 * \brief The given field must match the field under validation.
 *
 * The \a field under validation must have the same content as \a otherField.
 *
 * \note Unless \link Validator::validate() validation\endlink is started with \link Validator::NoTrimming NoTrimming\endlink,
 * whitespaces will be removed from the beginning and the end of the input value before validation.
 * If the \a field's value is empty or if the \a field is missing in the input data, the validation will succeed without
 * performing the validation itself. Use one of the \link ValidatorRequired required validators \endlink to require the
 * field to be present and not empty.
 *
 * \par Example
 * \code{.cpp}
 * void MyController::do_form(Context *c)
 * {
 *     Validator v({new ValidatorSame(QStringLiteral("field"),
 *                                    QStringLiteral("other_field"),
 *                                    QT_TRANSLATE_NOOP("MyController", "Other Field"),
 *                                    ValidatorMessages(QT_TRANSLATE_NOOP("MyController", "Field")))},
 *                  QLatin1String("MyController"));
 * }
 * \endcode
 *
 * \sa Validator for general usage of validators.
 */
class CUTELYST_PLUGIN_UTILS_VALIDATOR_EXPORT ValidatorSame : public ValidatorRule
{
public:
    /*!
     * \brief Constructs a new same validator.
     * \param field         Name of the input field to validate.
     * \param otherField    Name of the other field that must have the same input.
     * \param otherLabel    Human readable other field label, used for generic error messages.
     * \param messages      Custom error messages if validation fails.
     * \param defValKey     \link Context::stash() Stash \endlink key containing a default value if input field is empty. This value will \b NOT be validated.
     */
    ValidatorSame(const QString &field, const QString &otherField, const char *otherLabel = nullptr, const ValidatorMessages &messages = ValidatorMessages(), const QString &defValKey = QString());
    
    /*!
     * \brief Deconstructs the same validator.
     */
    ~ValidatorSame();
    
protected:
    /*!
     * \brief Performs the validation and returns the result.
     *
     * If validation succeeded, ValidatorReturnType::value will contain the input paramter
     * value as QString.
     */
    ValidatorReturnType validate(Context *c, const ParamsMultiMap &params) const override;

    /*!
     * \brief Returns a generic error message if validation failed.
     */
    QString genericValidationError(Context *c, const QVariant &errorData = QVariant()) const override;
    
private:
    Q_DECLARE_PRIVATE(ValidatorSame)
    Q_DISABLE_COPY(ValidatorSame)
};
    
}

#endif //CUTELYSTVALIDATORSAME_H

