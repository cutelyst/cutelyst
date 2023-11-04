/*
 * SPDX-FileCopyrightText: (C) 2018-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorfilesize_p.h"

#include <cmath>
#include <limits>

using namespace Cutelyst;

ValidatorFileSize::ValidatorFileSize(const QString &field,
                                     Option option,
                                     const QVariant &min,
                                     const QVariant &max,
                                     const ValidatorMessages &messages,
                                     const QString &defValKey)
    : ValidatorRule(*new ValidatorFileSizePrivate(field, option, min, max, messages, defValKey))
{
}

ValidatorFileSize::~ValidatorFileSize() = default;

bool ValidatorFileSize::validate(const QString &value,
                                 double min,
                                 double max,
                                 Cutelyst::ValidatorFileSize::Option option,
                                 const QLocale &locale,
                                 double *fileSize)
{
    bool valid = true;

    const QString str = value.simplified();
    QString digitPart;
    QString symbolPart;
    bool decimalPointFound = false;
    const QString decimalPoint(locale.decimalPoint());
    int multiplier     = 0;
    bool binary        = false;
    bool byteSignFound = false;
    ValidatorFileSizePrivate::StartsWith startsWith{ValidatorFileSizePrivate::StartsWith::NotSet};

    for (const QChar &ch : str) {
        if (valid) {
            const char16_t &uc = ch.toUpper().unicode();
            if (((uc >= ValidatorRulePrivate::ascii_0) && (uc <= ValidatorRulePrivate::ascii_9)) ||
                (ch == decimalPoint)) {
                if (startsWith == ValidatorFileSizePrivate::StartsWith::NotSet) {
                    startsWith = ValidatorFileSizePrivate::StartsWith::DigitPart;
                }
                if (ch == decimalPoint) {
                    if (decimalPointFound) {
                        qCDebug(C_VALIDATOR).nospace()
                            << "ValidatorFileSize: Validation failed for " << value << ": "
                            << "two decimal seperators in a row";
                        valid = false;
                        break;
                    } else {
                        decimalPointFound = true;
                    }
                }
                if ((symbolPart.isEmpty() &&
                     (startsWith == ValidatorFileSizePrivate::StartsWith::DigitPart)) ||
                    (!symbolPart.isEmpty() &&
                     (startsWith == ValidatorFileSizePrivate::StartsWith::SymbolPart))) {
                    digitPart.append(ch);
                } else {
                    qCDebug(C_VALIDATOR).nospace()
                        << "ValidatorFileSize: Validation failed for " << value << ": "
                        << "symbol inside digit part";
                    valid = false;
                    break;
                }
            } else if ((uc != ValidatorRulePrivate::asciiTab) &&
                       (uc != ValidatorRulePrivate::asciiSpace)) { // not a digit or decimal point
                                                                   // and not a space or tab
                if (startsWith == ValidatorFileSizePrivate::StartsWith::NotSet) {
                    startsWith = ValidatorFileSizePrivate::StartsWith::SymbolPart;
                }
                if ((digitPart.isEmpty() &&
                     (startsWith == ValidatorFileSizePrivate::StartsWith::SymbolPart)) ||
                    (!digitPart.isEmpty() &&
                     (startsWith == ValidatorFileSizePrivate::StartsWith::DigitPart))) {
                    switch (uc) {
                    case ValidatorFileSizePrivate::ascii_K:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol K already found";
                        } else {
                            multiplier = 1;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_M:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol M already found";
                        } else {
                            multiplier = 2;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_G:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol G already found";
                        } else {
                            multiplier = 3;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_T:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol T already found";
                        } else {
                            multiplier = 4;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_P:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol P already found";
                        } else {
                            multiplier = 5;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_E:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol E already found";
                        } else {
                            multiplier = 6;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorRulePrivate::ascii_Z:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol Z already found";
                        } else {
                            multiplier = 7;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_Y:
                    {
                        if (multiplier > 0) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "unit symbol Y already found";
                        } else {
                            multiplier = 8;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_I:
                    {
                        if ((multiplier == 0) || binary) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "binary indicator I already found or no unit symbol given "
                                   "before";
                        } else {
                            binary = true;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorFileSizePrivate::ascii_B:
                    {
                        if (byteSignFound) {
                            valid = false;
                            qCDebug(C_VALIDATOR).nospace()
                                << "ValdatorFileSize: Validation failed for " << value << ": "
                                << "byte symbol B already found";
                        } else {
                            byteSignFound = true;
                            symbolPart.append(ch);
                        }
                    } break;
                    case ValidatorRulePrivate::asciiTab:
                    case ValidatorRulePrivate::asciiSpace:
                        break;
                    default:
                        valid = false;
                        qCDebug(C_VALIDATOR).nospace()
                            << "ValdatorFileSize: Validation failed for " << value << ": "
                            << "invalid character in symbol part";
                        break;
                    }
                } else {
                    valid = false;
                    break;
                }
            }
        } else {
            break;
        }
    }

    if ((option == OnlyBinary) && !binary) {
        valid = false;
    } else if ((option == OnlyDecimal) && binary) {
        valid = false;
    } else if (option == ForceBinary) {
        binary = true;
    } else if (option == ForceDecimal) {
        binary = false;
    }

    if (valid) {
        bool ok     = false;
        double size = locale.toDouble(digitPart, &ok);
        if (!ok) {
            valid = false;
        } else {
            if (multiplier > 0) {
                const double _mult =
                    binary ? std::exp2(multiplier * 10) : std::pow(10.0, multiplier * 3);
                size *= _mult;
            }
            if ((min >= 1.0) && (size < min)) {
                valid = false;
            }
            if ((max >= 1.0) && (size > max)) {
                valid = false;
            }
            if (valid && fileSize) {
                *fileSize = size;
            }
        }
    }

    return valid;
}

ValidatorReturnType ValidatorFileSize::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorFileSize);

    const QString v = value(params);

    if (!v.isEmpty()) {

        double min = -1;
        double max = -1;
        bool ok    = true;
        if (d->min.isValid()) {
            min = d->extractDouble(c, params, d->min, &ok);
            if (!ok) {
                qCWarning(C_VALIDATOR).noquote()
                    << debugString(c) << "Invalid minimum size comparison data";
                result.errorMessage = validationDataError(c, 0);
            }
        }

        if (ok && d->max.isValid()) {
            max = d->extractDouble(c, params, d->max, &ok);
            if (!ok) {
                qCWarning(C_VALIDATOR).noquote()
                    << debugString(c) << "Invalid maximum size comparison data";
                result.errorMessage = validationDataError(c, 1);
            }
        }

        if (ok) {
            double size = 0;
            if (ValidatorFileSize::validate(v, min, max, d->option, c->locale(), &size)) {
                if (size < static_cast<double>(std::numeric_limits<qulonglong>::max())) {
                    result.value.setValue<qulonglong>(static_cast<qulonglong>(size + 0.5));
                } else {
                    result.value.setValue(size);
                }
            } else {
                result.errorMessage = validationError(c);
                qCWarning(C_VALIDATOR).noquote()
                    << debugString(c) << v << "is not a valid data size string";
            }
        }

    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorFileSize::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_D(const ValidatorFileSize);
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (d->min.isValid() || d->max.isValid()) {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorFileSize",
                                 "Invalid file size or file size not within the allowed limits.");
        } else {
            error = c->translate("Cutelyst::ValidatorFileSize",
                                 "The value in the “%1” field is either not a valid file size or "
                                 "not within the allowed limits.")
                        .arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorFileSize", "Invalid file size.");
        } else {
            error = c->translate("Cutelyst::ValidatorFileSize",
                                 "The “%1” field does not contain a valid file size.")
                        .arg(_label);
        }
    }

    return error;
}

QString ValidatorFileSize::genericValidationDataError(Context *c, const QVariant &errorData) const
{
    QString error;

    const QString _label = label(c);

    const int sizeType = errorData.toInt();

    if (sizeType == 0) { // minimum file size
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorFileSize",
                                 "The minimum file size comparison value is not valid.");
        } else {
            error = c->translate(
                         "Cutelyst::ValidatorFileSize",
                         "The minimum file size comparison value for the “%1” field is not valid.")
                        .arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorFileSize",
                                 "The maximum file size comparison value is not valid.");
        } else {
            error = c->translate(
                         "Cutelyst::ValidatorFileSize",
                         "The maximum file size comparison value for the “%1” field is not valid.")
                        .arg(_label);
        }
    }

    return error;
}

void ValidatorFileSize::inputPattern(Context *c, const QString &stashKey)
{
    Q_ASSERT(c);
    c->setStash(stashKey,
                c->locale().textDirection() == Qt::LeftToRight
                    ? QStringLiteral("^\\d+[,.٫]?\\d*\\s*[KkMmGgTt]?[Ii]?[Bb]?")
                    : QStringLiteral("[KkMmGgTt]?[Ii]?[Bb]?\\s*\\d+[,.٫]?\\d*"));
}
