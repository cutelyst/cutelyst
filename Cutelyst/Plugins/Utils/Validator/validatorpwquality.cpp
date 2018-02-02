/*
 * Copyright (C) 2018 Matthias Fehring <kontakt@buschmann23.de>
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

#include "validatorpwquality_p.h"
#include <pwquality.h>
#include <QLoggingCategory>

using namespace Cutelyst;

ValidatorPwQuality::ValidatorPwQuality(const QString &field, int threshold, const QVariant &options, const QString &userName, const QString &oldPassword, const ValidatorMessages &messages) :
    ValidatorRule(*new ValidatorPwQualityPrivate(field, threshold, options, userName, oldPassword, messages))
{
    // this is kind of a dirty hack for older versions of libpwquality
    // version 1.2.2 of libpwquality on Ubuntu Trusty for example will
    // return a score of 0 for the first time of use, not sure why
    // libpwquality 1.4.0 on openSUSE does not have this problem
    ValidatorPwQuality::validate(QStringLiteral("asdf234a"));
}

ValidatorPwQuality::~ValidatorPwQuality()
{

}

int ValidatorPwQuality::validate(const QString &value, const QVariant &options, const QString &oldPassword, const QString &user)
{
    int rv = 0;

    if (!value.isEmpty()) {

        pwquality_settings_t *pwq = pwquality_default_settings();
        if (pwq) {

            if (options.isValid()) {
                if (options.type() == QVariant::Map) {
                    const QVariantMap map = options.toMap();
                    auto i = map.constBegin();
                    while (i != map.constEnd()) {
                        const QString opt = i.key() + QLatin1Char('=') + i.value().toString();
                        rv = pwquality_set_option(pwq, opt.toUtf8().constData());
                        if (rv != 0) {
                            char buf[1024];
                            qCWarning(C_VALIDATOR, "ValidatorPwQuality: Failed to set pwquality option %s: %s", qUtf8Printable(opt), pwquality_strerror(buf, sizeof(buf), rv, nullptr));
                            break;
                        }
                        ++i;
                    }
                } else if (options.type() == QVariant::String) {
                    const QString configFile = options.toString();
                    void *auxerror;
                    rv = pwquality_read_config(pwq, configFile.toUtf8().constData(), &auxerror);
                    if (rv != 0) {
                        char buf[1024];
                        qCWarning(C_VALIDATOR, "ValidatorPwQuality: Failed to read configuration file: %s", pwquality_strerror(buf, sizeof(buf), rv, auxerror));
                    }
                }
            }

            if (rv == 0) {
                const char *pw = value.toUtf8().constData();
                const char *opw = oldPassword.isEmpty() ? nullptr : oldPassword.toUtf8().constData();
                const char *u = user.isEmpty() ? nullptr : user.toUtf8().constData();

                rv = pwquality_check(pwq, pw, opw, u, nullptr);
            }

            pwquality_free_settings(pwq);

        } else {
            rv = PWQ_ERROR_MEM_ALLOC;
        }
    } else {
        rv = PWQ_ERROR_EMPTY_PASSWORD;
    }

    return rv;
}

QString ValidatorPwQuality::errorString(Context *c, int returnValue, const QString &label, int threshold)
{
    QString error;

    if (label.isEmpty()) {
        switch (returnValue) {
        case PWQ_ERROR_MEM_ALLOC:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because of a memory allocation error.");
            break;
        case PWQ_ERROR_SAME_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password is the same as the old one.");
            break;
        case PWQ_ERROR_PALINDROME:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password is a palindrome.");
            break;
        case PWQ_ERROR_CASE_CHANGES_ONLY:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password differs with case changes only.");
            break;
        case PWQ_ERROR_TOO_SIMILAR:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password is too similar to the old one.");
            break;
        case PWQ_ERROR_USER_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains the user name in some form.");
            break;
        case PWQ_ERROR_GECOS_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains words from the real name of the user in some form.");
            break;
        case PWQ_ERROR_BAD_WORDS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains forbidden words in some form.");
            break;
        case PWQ_ERROR_MIN_DIGITS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains too few digits.");
            break;
        case PWQ_ERROR_MIN_UPPERS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains too few uppercase letters.");
            break;
        case PWQ_ERROR_MIN_LOWERS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains too few lowercase letters.");
            break;
        case PWQ_ERROR_MIN_OTHERS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains less than %ld non-alphanumeric characters.");
            break;
        case PWQ_ERROR_MIN_LENGTH:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password is too short.");
            break;
        case PWQ_ERROR_ROTATED:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password is just rotated old one.");
            break;
        case PWQ_ERROR_MIN_CLASSES:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password does not contain enough character classes.");
            break;
        case PWQ_ERROR_MAX_CONSECUTIVE:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains too many same characters consecutively.");
            break;
        case PWQ_ERROR_MAX_CLASS_REPEAT:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains too many characters of the same class consecutively.");
            break;
        case PWQ_ERROR_MAX_SEQUENCE:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password contains too long of a monotonic character sequence.");
            break;
        case PWQ_ERROR_EMPTY_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality", "No password supplied.");
            break;
        case PWQ_ERROR_RNG:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because we cannot obtain random numbers from the RNG device");
            break;
        case PWQ_ERROR_CRACKLIB_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password fails the dictionary check.");
            break;
        case PWQ_ERROR_UNKNOWN_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because of an unknown setting.");
            break;
        case PWQ_ERROR_INTEGER:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because of a bad integer value in the settings.");
            break;
        case PWQ_ERROR_NON_INT_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because of a settings entry is not of integer type.");
            break;
        case PWQ_ERROR_NON_STR_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because of a settings entry is not of string type.");
            break;
        case PWQ_ERROR_CFGFILE_OPEN:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because opening the configuration file failed.");
            break;
        case PWQ_ERROR_CFGFILE_MALFORMED:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because the configuration file is malformed.");
            break;
        case PWQ_ERROR_FATAL_FAILURE:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because of a fatal failure.");
            break;
        default:
        {
            if (returnValue < 0) {
                error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check failed because of an unknown error.");
            } else {
                if (returnValue < threshold) {
                    error = c->translate("Cutelyst::ValidatorPwQuality", "The password quality score of %1 is below the threshold of %2.").arg(QString::number(returnValue), QString::number(threshold));
                }
            }
        }
            break;
        }
    } else {
        switch (returnValue) {
        case PWQ_ERROR_MEM_ALLOC:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because of a memory allocation error.").arg(label);
            break;
        case PWQ_ERROR_SAME_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field is the same as the old one.").arg(label);
            break;
        case PWQ_ERROR_PALINDROME:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field is a palindrome.").arg(label);
            break;
        case PWQ_ERROR_CASE_CHANGES_ONLY:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field differs with case changes only.").arg(label);
            break;
        case PWQ_ERROR_TOO_SIMILAR:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field is too similar to the old one.").arg(label);
            break;
        case PWQ_ERROR_USER_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains the user name in some form.").arg(label);
            break;
        case PWQ_ERROR_GECOS_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains words from the real name of the user in some form.").arg(label);
            break;
        case PWQ_ERROR_BAD_WORDS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains forbidden words in some form.").arg(label);
            break;
        case PWQ_ERROR_MIN_DIGITS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains too few digits.").arg(label);
            break;
        case PWQ_ERROR_MIN_UPPERS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains too few uppercase letters.").arg(label);
            break;
        case PWQ_ERROR_MIN_LOWERS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains too few lowercase letters.").arg(label);
            break;
        case PWQ_ERROR_MIN_OTHERS:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains less than %ld non-alphanumeric characters.").arg(label);
            break;
        case PWQ_ERROR_MIN_LENGTH:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field is too short.").arg(label);
            break;
        case PWQ_ERROR_ROTATED:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field is just rotated old one.").arg(label);
            break;
        case PWQ_ERROR_MIN_CLASSES:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field does not contain enough character classes.").arg(label);
            break;
        case PWQ_ERROR_MAX_CONSECUTIVE:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains too many same characters consecutively.").arg(label);
            break;
        case PWQ_ERROR_MAX_CLASS_REPEAT:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains too many characters of the same class consecutively.").arg(label);
            break;
        case PWQ_ERROR_MAX_SEQUENCE:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field contains too long of a monotonic character sequence.").arg(label);
            break;
        case PWQ_ERROR_EMPTY_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality", "No password supplied in the “%1” field.").arg(label);
            break;
        case PWQ_ERROR_RNG:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because we cannot obtain random numbers from the RNG device").arg(label);
            break;
        case PWQ_ERROR_CRACKLIB_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password in the “%1” field fails the dictionary check.").arg(label);
            break;
        case PWQ_ERROR_UNKNOWN_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because of an unknown setting.").arg(label);
            break;
        case PWQ_ERROR_INTEGER:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because of a bad integer value in the settings.").arg(label);
            break;
        case PWQ_ERROR_NON_INT_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because of a settings entry is not of integer type.").arg(label);
            break;
        case PWQ_ERROR_NON_STR_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because of a settings entry is not of string type.").arg(label);
            break;
        case PWQ_ERROR_CFGFILE_OPEN:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because opening the configuration file failed.").arg(label);
            break;
        case PWQ_ERROR_CFGFILE_MALFORMED:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because the configuration file is malformed.").arg(label);
            break;
        case PWQ_ERROR_FATAL_FAILURE:
            error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1“ field failed because of a fatal failure.").arg(label);
            break;
        default:
        {
            if (returnValue < 0) {
                error = c->translate("Cutelyst::ValidatorPwQuality", "Password quality check for the “%1” field failed because of an unknown error.").arg(label);
            } else {
                if (returnValue < threshold) {
                    error = c->translate("Cutelyst::ValidatorPwQuality", "The quality score of %1 for the password in the “%2” field is below the threshold of %3.").arg(QString::number(returnValue), label, QString::number(threshold));
                }
            }
        }
            break;
        }
    }

    return error;
}

ValidatorReturnType ValidatorPwQuality::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        Q_D(const ValidatorPwQuality);
        QVariant opts;
        if (d->options.isValid()) {
            if (d->options.type() == QVariant::Map) {
                opts = d->options;
            } else if (d->options.type() == QVariant::String) {
                const QString optString = d->options.toString();
                if (c->stash().contains(optString)) {
                    opts = c->stash(optString);
                } else {
                    opts = d->options;
                }
            }
        }
        QString un;
        if (!d->userName.isEmpty()) {
            un = params.value(d->userName);
            if (un.isEmpty()) {
                un = c->stash(d->userName).toString();
            }
        }
        QString opw;
        if (!d->oldPassword.isEmpty()) {
            opw = params.value(d->oldPassword);
            if (opw.isEmpty()) {
                opw = c->stash(d->oldPassword).toString();
            }
        }
        int rv = validate(v, opts, opw, un);
        if (rv < d->threshold) {
            result.errorMessage = validationError(c, rv);
            if (C_VALIDATOR().isDebugEnabled()) {
                if (rv < 0) {
                    char buf[1024];
                    qCDebug(C_VALIDATOR, "ValidatorPwQuality: Validation failed for field %s at %s::%s: %s", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), pwquality_strerror(buf, sizeof(buf), rv, nullptr));
                } else {
                    qCDebug(C_VALIDATOR, "ValidatorPwQuality: Validation failed for field %s at %s::%s because the quality score %i is below the threshold of %i.", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()), rv, d->threshold);
                }
            }
        } else {
            result.value.setValue<QString>(v);
        }
    }

    return result;
}

QString ValidatorPwQuality::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;

    Q_D(const ValidatorPwQuality);
    const int returnValue = errorData.toInt();
    const QString _label = label(c);
    error = ValidatorPwQuality::errorString(c, returnValue, _label, d->threshold);

    return error;
}
