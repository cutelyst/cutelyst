/*
 * SPDX-FileCopyrightText: (C) 2018-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorpwquality_p.h"

#include <pwquality.h>

#include <QLoggingCategory>

using namespace Cutelyst;

ValidatorPwQuality::ValidatorPwQuality(const QString &field,
                                       int threshold,
                                       const QVariant &options,
                                       const QString &userName,
                                       const QString &oldPassword,
                                       const ValidatorMessages &messages)
    : ValidatorRule(*new ValidatorPwQualityPrivate(field,
                                                   threshold,
                                                   options,
                                                   userName,
                                                   oldPassword,
                                                   messages))
{
}

ValidatorPwQuality::~ValidatorPwQuality() = default;

int ValidatorPwQuality::validate(const QString &value,
                                 const QVariant &options,
                                 const QString &oldPassword,
                                 const QString &user)
{
    int rv = 0;

    if (!value.isEmpty()) {

        pwquality_settings_t *pwq = pwquality_default_settings();
        if (pwq) {

            bool optionsSet = false;
            if (options.isValid()) {
                if (options.typeId() == QMetaType::QVariantMap) {
                    const QVariantMap map = options.toMap();
                    if (!map.empty()) {
                        auto i = map.constBegin();
                        while (i != map.constEnd()) {
                            const QString opt = i.key() + QLatin1Char('=') + i.value().toString();
                            const int orv     = pwquality_set_option(pwq, opt.toUtf8().constData());
                            if (orv != 0) {
                                QList<char> buf(ValidatorPwQualityPrivate::errStrBufSize);
                                qCWarning(C_VALIDATOR).noquote().nospace()
                                    << "ValidatorPwQuality: Failed to set pwquality option " << opt
                                    << ": "
                                    << pwquality_strerror(buf.data(), buf.size(), orv, nullptr);
                            }
                            ++i;
                        }
                        optionsSet = true;
                    }
                } else if (options.typeId() == QMetaType::QString) {
                    const QString configFile = options.toString();
                    if (!configFile.isEmpty()) {
                        if (C_VALIDATOR().isWarningEnabled()) {
                            void *auxerror = nullptr;
                            const int rcrv = pwquality_read_config(
                                pwq, configFile.toUtf8().constData(), &auxerror);
                            if (rcrv != 0) {
                                QList<char> buf(ValidatorPwQualityPrivate::errStrBufSize);
                                qCWarning(C_VALIDATOR).noquote().nospace()
                                    << "ValidatorPwQuality: Failed to read configuration file "
                                    << configFile << ": "
                                    << pwquality_strerror(buf.data(), buf.size(), rcrv, auxerror);
                            }
                        } else {
                            pwquality_read_config(pwq, configFile.toUtf8().constData(), nullptr);
                        }
                        optionsSet = true;
                    }
                }
            }

            if (!optionsSet) {
                if (C_VALIDATOR().isWarningEnabled()) {
                    void *auxerror = nullptr;
                    const int rcrv = pwquality_read_config(pwq, nullptr, &auxerror);
                    if (rcrv != 0) {
                        QList<char> buf(ValidatorPwQualityPrivate::errStrBufSize);
                        qCWarning(C_VALIDATOR).noquote()
                            << "VaidatorPwQuality: Failed to read default configuration file:"
                            << pwquality_strerror(buf.data(), buf.size(), rcrv, auxerror);
                    }
                } else {
                    pwquality_read_config(pwq, nullptr, nullptr);
                }
            }

            const QByteArray pwba  = value.toUtf8();
            const char *pw         = pwba.constData();
            const QByteArray opwba = oldPassword.toUtf8();
            const char *opw        = opwba.isEmpty() ? nullptr : opwba.constData();
            const QByteArray uba   = user.toUtf8();
            const char *u          = uba.isEmpty() ? nullptr : uba.constData();

            rv = pwquality_check(pwq, pw, opw, u, nullptr);

            pwquality_free_settings(pwq);

        } else {
            rv = PWQ_ERROR_MEM_ALLOC;
        }
    } else {
        rv = PWQ_ERROR_EMPTY_PASSWORD;
    }

    return rv;
}

QString ValidatorPwQuality::errorString(Context *c,
                                        int returnValue,
                                        const QString &label,
                                        int threshold)
{
    if (label.isEmpty()) {
        switch (returnValue) {
        case PWQ_ERROR_MEM_ALLOC:
            //% "Password quality check failed because of a memory allocation error."
            return c->qtTrId("cutelyst-valpwq-err-memalloc");
        case PWQ_ERROR_SAME_PASSWORD:
            //% "The password is the same as the old one."
            return c->qtTrId("cutelyst-valpwq-err-samepass");
        case PWQ_ERROR_PALINDROME:
            //% "The password is a palindrome."
            return c->qtTrId("cutelyst-valpwq-err-palindrome");
        case PWQ_ERROR_CASE_CHANGES_ONLY:
            //% "The password differs with case changes only from the old one."
            return c->qtTrId("cutelyst-valpwq-err-casechangesonly");
        case PWQ_ERROR_TOO_SIMILAR:
            //% "The password is too similar to the old one."
            return c->qtTrId("cutelyst-valpwq-err-toosimilar");
        case PWQ_ERROR_USER_CHECK:
            //% "The password contains the user name in some form."
            return c->qtTrId("cutelyst-valpwq-err-usercheck");
        case PWQ_ERROR_GECOS_CHECK:
            //% "The password contains words from the real name of the user in some form."
            return c->qtTrId("cutelyst-valpwq-err-gecoscheck");
        case PWQ_ERROR_BAD_WORDS:
            //% "The password contains forbidden words in some form."
            return c->qtTrId("cutelyst-valpwq-err-badwords");
        case PWQ_ERROR_MIN_DIGITS:
            //% "The password does not contain enough digits."
            return c->qtTrId("cutelyst-valpwq-err-mindigits");
        case PWQ_ERROR_MIN_UPPERS:
            //% "The password does not contain enough uppercase letters."
            return c->qtTrId("cutelyst-valpwq-err-minuppers");
        case PWQ_ERROR_MIN_LOWERS:
            //% "The password does not contain enough lowercase letters."
            return c->qtTrId("cutelyst-valpwq-err-minlowers");
        case PWQ_ERROR_MIN_OTHERS:
            //% "The password does not contain enough non-alphanumeric characters."
            return c->qtTrId("cutelyst-valpwq-err-minothers");
        case PWQ_ERROR_MIN_LENGTH:
            //% "The password is too short."
            return c->qtTrId("cutelyst-valpwq-err-minlength");
        case PWQ_ERROR_ROTATED:
            //% "The password is just the rotated old one."
            return c->qtTrId("cutelyst-valpwq-err-rotated");
        case PWQ_ERROR_MIN_CLASSES:
            //% "The password does not contain enough different character types."
            return c->qtTrId("cutelyst-valpwq-err-minclasses");
        case PWQ_ERROR_MAX_CONSECUTIVE:
            //% "The password contains too many same characters consecutively."
            return c->qtTrId("cutelyst-valpwq-err-maxconsecutive");
        case PWQ_ERROR_MAX_CLASS_REPEAT:
            //% "The password contains too many characters of the same type consecutively."
            return c->qtTrId("cutelyst-valpwq-err-maxclassrepeat");
        case PWQ_ERROR_MAX_SEQUENCE:
            //% "The password contains too long a monotonous string."
            return c->qtTrId("cutelyst-valpwq-err-maxsequence");
        case PWQ_ERROR_EMPTY_PASSWORD:
            //% "No password supplied."
            return c->qtTrId("cutelyst-valpwq-err-emptypw");
        case PWQ_ERROR_RNG:
            //% "Password quality check failed because we cannot obtain random "
            //% "numbers from the RNG device."
            return c->qtTrId("cutelyst-valpwq-err-rng");
        case PWQ_ERROR_CRACKLIB_CHECK:
            //% "The password fails the dictionary check."
            return c->qtTrId("cutelyst-valpwq-err-cracklibcheck");
        case PWQ_ERROR_UNKNOWN_SETTING:
            //% "Password quality check failed because of an unknown setting."
            return c->qtTrId("cutelyst-valpwq-err-unknownsetting");
        case PWQ_ERROR_INTEGER:
            //% "Password quality check failed because of a bad integer value in the settings."
            return c->qtTrId("cutelyst-valpwq-err-integer");
        case PWQ_ERROR_NON_INT_SETTING:
            //% "Password quality check failed because of a settings entry is not "
            //% "of integer type."
            return c->qtTrId("cutelyst-valpwq-err-nonintsetting");
        case PWQ_ERROR_NON_STR_SETTING:
            //% "Password quality check failed because of a settings entry is not of string type."
            return c->qtTrId("cutelyst-valpwq-err-nonstrsetting");
        case PWQ_ERROR_CFGFILE_OPEN:
            //% "Password quality check failed because opening the configuration file failed."
            return c->qtTrId("cutelyst-valpwq-err-cfgfileopen");
        case PWQ_ERROR_CFGFILE_MALFORMED:
            //% "Password quality check failed because the configuration file is malformed."
            return c->qtTrId("cutelyst-valpwq-err-cfgfilemalformed");
        case PWQ_ERROR_FATAL_FAILURE:
            //% "Password quality check failed because of a fatal failure."
            return c->qtTrId("cutelyst-valpwq-err-fatalfailure");
        default:
        {
            if (returnValue < 0) {
                //% "Password quality check failed because of an unknown error."
                return c->qtTrId("cutelyst-valpwq-err-unknown");
            } else {
                if (returnValue < threshold) {
                    //% "The password quality score of %1 is below the threshold of %2."
                    return c->qtTrId("cutelyst-valpwq-err-belowthreshold")
                        .arg(QString::number(returnValue), QString::number(threshold));
                } else {
                    return {};
                }
            }
        }
        }
    } else {
        switch (returnValue) {
        case PWQ_ERROR_MEM_ALLOC:
            //% "Password quality check for the “%1“ field failed because of a "
            //% "memory allocation error."
            return c->qtTrId("cutelyst-valpwq-err-memalloc-label").arg(label);
        case PWQ_ERROR_SAME_PASSWORD:
            //% "The password in the “%1” field is the same as the old one."
            return c->qtTrId("cutelyst-valpwq-err-samepass-label").arg(label);
        case PWQ_ERROR_PALINDROME:
            //% "The password in the “%1” field is a palindrome."
            return c->qtTrId("cutelyst-valpwq-err-palindrome-label").arg(label);
        case PWQ_ERROR_CASE_CHANGES_ONLY:
            //% "The password in the “%1” field differs with case changes only from the old one."
            return c->qtTrId("cutelyst-valpwq-err-casechangesonly-label").arg(label);
        case PWQ_ERROR_TOO_SIMILAR:
            //% "The password in the “%1” field is too similar to the old one."
            return c->qtTrId("cutelyst-valpwq-err-toosimilar-label").arg(label);
        case PWQ_ERROR_USER_CHECK:
            //% "The password in the “%1” field contains the user name in some form."
            return c->qtTrId("cutelyst-valpwq-err-usercheck-label").arg(label);
        case PWQ_ERROR_GECOS_CHECK:
            //% "The password in the “%1” field contains words from the real name "
            //% "of the user name in some form."
            return c->qtTrId("cutelyst-valpwq-err-gecoscheck-label").arg(label);
        case PWQ_ERROR_BAD_WORDS:
            //% "The password in the “%1” field contains forbidden words in some form."
            return c->qtTrId("cutelyst-valpwq-err-badwords-label").arg(label);
        case PWQ_ERROR_MIN_DIGITS:
            //% "The password in the “%1” field does not contain enough digits."
            return c->qtTrId("cutelyst-valpwq-err-mindigits-label").arg(label);
        case PWQ_ERROR_MIN_UPPERS:
            //% "The password in the “%1” field does not contain enough uppercase letters."
            return c->qtTrId("cutelyst-valpwq-err-minuppers-label").arg(label);
        case PWQ_ERROR_MIN_LOWERS:
            //% "The password in the “%1” field does not contain enough lowercase letters."
            return c->qtTrId("cutelyst-valpwq-err-minlowers-label").arg(label);
        case PWQ_ERROR_MIN_OTHERS:
            //% "The password in the “%1” field does not contain enough non-alphanumeric "
            //% "characters."
            return c->qtTrId("cutelyst-valpwq-err-minothers-label").arg(label);
        case PWQ_ERROR_MIN_LENGTH:
            //% "The password in the “%1” field is too short."
            return c->qtTrId("cutelyst-valpwq-err-minlength-label").arg(label);
        case PWQ_ERROR_ROTATED:
            //% "The password in the “%1” field is just the rotated old one."
            return c->qtTrId("cutelyst-valpwq-err-rotated-label").arg(label);
        case PWQ_ERROR_MIN_CLASSES:
            //% "The password in the “%1” field does not contain enough character types."
            return c->qtTrId("cutelyst-valpwq-err-minclasses-label").arg(label);
        case PWQ_ERROR_MAX_CONSECUTIVE:
            //% "The password in the “%1” field contains too many same characters "
            //% "consecutively."
            return c->qtTrId("cutelyst-valpwq-err-maxconsecutive-label").arg(label);
        case PWQ_ERROR_MAX_CLASS_REPEAT:
            //% "The password in the “%1” field contains too many characters of "
            //% "the same type consecutively."
            return c->qtTrId("cutelyst-valpwq-err-maxclassrepeat-label").arg(label);
        case PWQ_ERROR_MAX_SEQUENCE:
            //% "The password in the “%1” field contains contains too long a "
            //% "monotonous string."
            return c->qtTrId("cutelyst-valpwq-err-maxsequence-label").arg(label);
        case PWQ_ERROR_EMPTY_PASSWORD:
            //% "No password supplied in the “%1” field."
            return c->qtTrId("cutelyst-valpwq-err-emptypw-label").arg(label);
        case PWQ_ERROR_RNG:
            //% "Password quality check for the “%1“ field failed because we "
            //% "cannot obtain random numbers from the RNG device."
            return c->qtTrId("cutelyst-valpwq-err-rng-label").arg(label);
        case PWQ_ERROR_CRACKLIB_CHECK:
            //% "The password in the “%1” field fails the dictionary check."
            return c->qtTrId("cutelyst-valpwq-err-cracklibcheck-label").arg(label);
        case PWQ_ERROR_UNKNOWN_SETTING:
            //% "Password quality check for the “%1“ field failed because of an "
            //% "unknown setting."
            return c->qtTrId("cutelyst-valpwq-err-unknownsetting-label").arg(label);
        case PWQ_ERROR_INTEGER:
            //% "Password quality check for the “%1“ field failed because of a "
            //% "bad integer value in the settings."
            return c->qtTrId("cutelyst-valpwq-err-integer-label").arg(label);
        case PWQ_ERROR_NON_INT_SETTING:
            //% "Password quality check for the “%1“ field failed because of a "
            //% "settings entry is not of integer type."
            return c->qtTrId("cutelyst-valpwq-err-nonintsetting-label").arg(label);
        case PWQ_ERROR_NON_STR_SETTING:
            //% "Password quality check for the “%1“ field failed because of a "
            //% "settings entry is not of string type."
            return c->qtTrId("cutelyst-valpwq-err-nonstrsetting-label").arg(label);
        case PWQ_ERROR_CFGFILE_OPEN:
            //% "Password quality check for the “%1“ field failed because opening "
            //% "the configuration file failed."
            return c->qtTrId("cutelyst-valpwq-err-cfgfileopen-label").arg(label);
        case PWQ_ERROR_CFGFILE_MALFORMED:
            //% "Password quality check for the “%1“ field failed because the "
            //% "configuration file is malformed."
            return c->qtTrId("cutelyst-valpwq-err-cfgfilemalformed-label").arg(label);
        case PWQ_ERROR_FATAL_FAILURE:
            //% "Password quality check for the “%1“ field failed because of a fatal failure."
            return c->qtTrId("cutelyst-valpwq-err-fatalfailure-label").arg(label);
        default:
        {
            if (returnValue < 0) {
                //% "Password quality check for the “%1” field failed because of "
                //% "an unknown error."
                return c->qtTrId("cutelyst-valpwq-err-unknown-label").arg(label);
            } else {
                if (returnValue < threshold) {
                    //% "The quality score of %1 for the password in the “%2” field "
                    //% "is below the threshold of %3."
                    return c->qtTrId("cutelyst-valpwq-err-belowthreshold-label")
                        .arg(QString::number(returnValue), QString::number(threshold));
                } else {
                    return {};
                }
            }
        }
        }
    }
}

ValidatorReturnType ValidatorPwQuality::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    const QString v = value(params);

    if (!v.isEmpty()) {
        Q_D(const ValidatorPwQuality);
        QVariant opts;
        if (d->options.isValid()) {
            if (d->options.typeId() == QMetaType::QVariantMap) {
                opts = d->options;
            } else if (d->options.typeId() == QMetaType::QString) {
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
                    QList<char> buf(ValidatorPwQualityPrivate::errStrBufSize);
                    qCDebug(C_VALIDATOR).noquote()
                        << debugString(c)
                        << pwquality_strerror(buf.data(), buf.size(), rv, nullptr);
                } else {
                    qCDebug(C_VALIDATOR).noquote() << debugString(c) << "The quality score" << rv
                                                   << "is below the threshold of" << d->threshold;
                }
            }
        } else {
            qCDebug(C_VALIDATOR).noquote()
                << "ValidatorPwQuality: \"" << v << "\" got a quality score of" << rv;
            result.value = v;
        }
    }

    return result;
}

QString ValidatorPwQuality::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_D(const ValidatorPwQuality);
    return ValidatorPwQuality::errorString(c, errorData.toInt(), label(c), d->threshold);
}
