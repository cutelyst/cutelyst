/*
 * SPDX-FileCopyrightText: (C) 2018-2022 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorfilesize_p.h"

#include <cmath>
#include <limits>

using namespace Cutelyst;

ValidatorFileSize::ValidatorFileSize(const QString &field, Option option, const QVariant &min, const QVariant &max, const ValidatorMessages &messages, const QString &defValKey)
    : ValidatorRule(*new ValidatorFileSizePrivate(field, option, min, max, messages, defValKey))
{
}

ValidatorFileSize::~ValidatorFileSize()
{
}

bool ValidatorFileSize::validate(const QString &value, double min, double max, Cutelyst::ValidatorFileSize::Option option, const QLocale &locale, double *fileSize)
{
    bool valid = true;

    QString digitPart;
    QString symbolPart;
    bool decimalPointFound = false;
    const QString decimalPoint(locale.decimalPoint());
    int multiplier     = 0;
    bool binary        = false;
    bool byteSignFound = false;
    qint8 startsWith   = 0; // 0 not set, -1 digit part, 1 symbol part

    for (const QChar &ch : value) {
        if (valid) {
            const ushort &uc = ch.unicode();
            if (((uc > 47) && (uc < 58)) || (ch == decimalPoint)) {
                if (startsWith == 0) {
                    startsWith = -1;
                }
                if (ch == decimalPoint) {
                    if (decimalPointFound) {
                        valid = false;
                        break;
                    } else {
                        decimalPointFound = true;
                    }
                }
                if ((symbolPart.isEmpty() && (startsWith < 0)) || (!symbolPart.isEmpty() && (startsWith > 0))) {
                    digitPart.append(ch);
                } else {
                    valid = false;
                    break;
                }
            } else if ((uc != 9) && (uc != 32)) { // not a digit or decimal point and not a space or tab
                if (startsWith == 0) {
                    startsWith = 1;
                }
                if ((digitPart.isEmpty() && (startsWith > 0)) || (!digitPart.isEmpty() && (startsWith < 0))) {
                    switch (uc) {
                    case 75:  // K
                    case 107: // k
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 1;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 77:  // M
                    case 109: // m
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 2;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 71:  // G
                    case 103: // g
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 3;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 84:  // T
                    case 116: // t
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 4;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 80:  // P
                    case 112: // p
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 5;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 69:  // E
                    case 101: // e
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 6;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 90:  // Z
                    case 122: // z
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 7;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 89:  // Y
                    case 121: // y
                    {
                        if (multiplier > 0) {
                            valid = false;
                        } else {
                            multiplier = 8;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 73:  // I
                    case 105: // i
                    {
                        if ((multiplier == 0) || binary) {
                            valid = false;
                        } else {
                            binary = true;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 66: // B
                    case 98: // b
                    {
                        if (byteSignFound) {
                            valid = false;
                        } else {
                            byteSignFound = true;
                            symbolPart.append(ch);
                        }
                    } break;
                    case 9:  // horizontal tab
                    case 32: // space
                        break;
                    default:
                        valid = false;
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
                const double _mult = binary ? std::exp2(multiplier * 10) : std::pow(10.0, multiplier * 3);
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
                result.errorMessage = validationDataError(c, 0);
            }
        }

        if (ok && d->max.isValid()) {
            max = d->extractDouble(c, params, d->max, &ok);
            if (!ok) {
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
            }
        }

    } else {
        defaultValue(c, &result, "ValidatorFileSize");
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
            error = c->translate("Cutelyst::ValidatorFileSize", "Invalid file size or file size not within the allowed limits.");
        } else {
            error = c->translate("Cutelyst::ValidatorFileSize", "The value in the “%1” field is either not a valid file size or not within the allowed limits.").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorFileSize", "Invalid file size.");
        } else {
            error = c->translate("Cutelyst::ValidatorFileSize", "The “%1” field does not contain a valid file size.").arg(_label);
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
            error = c->translate("Cutelyst::ValidatorFileSize", "The minimum file size comparison value is not valid.");
        } else {
            error = c->translate("Cutelyst::ValidatorFileSize", "The minimum file size comparison value for the “%1” field is not valid.").arg(_label);
        }
    } else {
        if (_label.isEmpty()) {
            error = c->translate("Cutelyst::ValidatorFileSize", "The maximum file size comparison value is not valid.");
        } else {
            error = c->translate("Cutelyst::ValidatorFileSize", "The maximum file size comparison value for the “%1” field is not valid.").arg(_label);
        }
    }

    return error;
}

void ValidatorFileSize::inputPattern(Context *c, const QString &stashKey)
{
    Q_ASSERT(c);
    c->setStash(stashKey, c->locale().textDirection() == Qt::LeftToRight ? QStringLiteral("^\\d+[,.٫]?\\d*\\s*[KkMmGgTt]?[Ii]?[Bb]?") : QStringLiteral("[KkMmGgTt]?[Ii]?[Bb]?\\s*\\d+[,.٫]?\\d*"));
}
