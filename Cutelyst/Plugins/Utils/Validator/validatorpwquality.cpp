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
    QString error;

    if (label.isEmpty()) {
        switch (returnValue) {
        case PWQ_ERROR_MEM_ALLOC:
            error =
                c->translate("Cutelyst::ValidatorPwQuality",
                             "Password quality check failed because of a memory allocation error.");
            break;
        case PWQ_ERROR_SAME_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password is the same as the old one.");
            break;
        case PWQ_ERROR_PALINDROME:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password is a palindrome.");
            break;
        case PWQ_ERROR_CASE_CHANGES_ONLY:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password differs with case changes only.");
            break;
        case PWQ_ERROR_TOO_SIMILAR:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password is too similar to the old one.");
            break;
        case PWQ_ERROR_USER_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains the user name in some form.");
            break;
        case PWQ_ERROR_GECOS_CHECK:
            error = c->translate(
                "Cutelyst::ValidatorPwQuality",
                "The password contains words from the real name of the user in some form.");
            break;
        case PWQ_ERROR_BAD_WORDS:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains forbidden words in some form.");
            break;
        case PWQ_ERROR_MIN_DIGITS:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains too few digits.");
            break;
        case PWQ_ERROR_MIN_UPPERS:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains too few uppercase letters.");
            break;
        case PWQ_ERROR_MIN_LOWERS:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains too few lowercase letters.");
            break;
        case PWQ_ERROR_MIN_OTHERS:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains too few non-alphanumeric characters.");
            break;
        case PWQ_ERROR_MIN_LENGTH:
            error = c->translate("Cutelyst::ValidatorPwQuality", "The password is too short.");
            break;
        case PWQ_ERROR_ROTATED:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password is just the rotated old one.");
            break;
        case PWQ_ERROR_MIN_CLASSES:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password does not contain enough different character types.");
            break;
        case PWQ_ERROR_MAX_CONSECUTIVE:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains too many same characters consecutively.");
            break;
        case PWQ_ERROR_MAX_CLASS_REPEAT:
            error = c->translate(
                "Cutelyst::ValidatorPwQuality",
                "The password contains too many characters of the same type consecutively.");
            break;
        case PWQ_ERROR_MAX_SEQUENCE:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password contains too long a monotonous string.");
            break;
        case PWQ_ERROR_EMPTY_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality", "No password supplied.");
            break;
        case PWQ_ERROR_RNG:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check failed because we cannot obtain random "
                                 "numbers from the RNG device.");
            break;
        case PWQ_ERROR_CRACKLIB_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password fails the dictionary check.");
            break;
        case PWQ_ERROR_UNKNOWN_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check failed because of an unknown setting.");
            break;
        case PWQ_ERROR_INTEGER:
            error = c->translate(
                "Cutelyst::ValidatorPwQuality",
                "Password quality check failed because of a bad integer value in the settings.");
            break;
        case PWQ_ERROR_NON_INT_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check failed because of a settings entry is not "
                                 "of integer type.");
            break;
        case PWQ_ERROR_NON_STR_SETTING:
            error = c->translate(
                "Cutelyst::ValidatorPwQuality",
                "Password quality check failed because of a settings entry is not of string type.");
            break;
        case PWQ_ERROR_CFGFILE_OPEN:
            error = c->translate(
                "Cutelyst::ValidatorPwQuality",
                "Password quality check failed because opening the configuration file failed.");
            break;
        case PWQ_ERROR_CFGFILE_MALFORMED:
            error = c->translate(
                "Cutelyst::ValidatorPwQuality",
                "Password quality check failed because the configuration file is malformed.");
            break;
        case PWQ_ERROR_FATAL_FAILURE:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check failed because of a fatal failure.");
            break;
        default:
        {
            if (returnValue < 0) {
                error = c->translate("Cutelyst::ValidatorPwQuality",
                                     "Password quality check failed because of an unknown error.");
            } else {
                if (returnValue < threshold) {
                    error = c->translate(
                                 "Cutelyst::ValidatorPwQuality",
                                 "The password quality score of %1 is below the threshold of %2.")
                                .arg(QString::number(returnValue), QString::number(threshold));
                }
            }
        } break;
        }
    } else {
        switch (returnValue) {
        case PWQ_ERROR_MEM_ALLOC:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because of a "
                                 "memory allocation error.")
                        .arg(label);
            break;
        case PWQ_ERROR_SAME_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field is the same as the old one.")
                        .arg(label);
            break;
        case PWQ_ERROR_PALINDROME:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field is a palindrome.")
                        .arg(label);
            break;
        case PWQ_ERROR_CASE_CHANGES_ONLY:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field differs with case changes only.")
                        .arg(label);
            break;
        case PWQ_ERROR_TOO_SIMILAR:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field is too similar to the old one.")
                        .arg(label);
            break;
        case PWQ_ERROR_USER_CHECK:
            error =
                c->translate("Cutelyst::ValidatorPwQuality",
                             "The password in the “%1” field contains the user name in some form.")
                    .arg(label);
            break;
        case PWQ_ERROR_GECOS_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field contains words from the real name "
                                 "of the user name in some form.")
                        .arg(label);
            break;
        case PWQ_ERROR_BAD_WORDS:
            error = c->translate(
                         "Cutelyst::ValidatorPwQuality",
                         "The password in the “%1” field contains forbidden words in some form.")
                        .arg(label);
            break;
        case PWQ_ERROR_MIN_DIGITS:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field contains too few digits.")
                        .arg(label);
            break;
        case PWQ_ERROR_MIN_UPPERS:
            error =
                c->translate("Cutelyst::ValidatorPwQuality",
                             "The password in the “%1” field contains too few uppercase letters.")
                    .arg(label);
            break;
        case PWQ_ERROR_MIN_LOWERS:
            error =
                c->translate("Cutelyst::ValidatorPwQuality",
                             "The password in the “%1” field contains too few lowercase letters.")
                    .arg(label);
            break;
        case PWQ_ERROR_MIN_OTHERS:
            error =
                c->translate(
                     "Cutelyst::ValidatorPwQuality",
                     "The password in the “%1” field contains too few non-alphanumeric characters.")
                    .arg(label);
            break;
        case PWQ_ERROR_MIN_LENGTH:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field is too short.")
                        .arg(label);
            break;
        case PWQ_ERROR_ROTATED:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field is just the rotated old one.")
                        .arg(label);
            break;
        case PWQ_ERROR_MIN_CLASSES:
            error = c->translate(
                         "Cutelyst::ValidatorPwQuality",
                         "The password in the “%1” field does not contain enough character types.")
                        .arg(label);
            break;
        case PWQ_ERROR_MAX_CONSECUTIVE:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field contains too many same characters "
                                 "consecutively.")
                        .arg(label);
            break;
        case PWQ_ERROR_MAX_CLASS_REPEAT:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field contains too many characters of "
                                 "the same type consecutively.")
                        .arg(label);
            break;
        case PWQ_ERROR_MAX_SEQUENCE:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field contains contains too long a "
                                 "monotonous string.")
                        .arg(label);
            break;
        case PWQ_ERROR_EMPTY_PASSWORD:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "No password supplied in the “%1” field.")
                        .arg(label);
            break;
        case PWQ_ERROR_RNG:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because we "
                                 "cannot obtain random numbers from the RNG device.")
                        .arg(label);
            break;
        case PWQ_ERROR_CRACKLIB_CHECK:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "The password in the “%1” field fails the dictionary check.")
                        .arg(label);
            break;
        case PWQ_ERROR_UNKNOWN_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because of an "
                                 "unknown setting.")
                        .arg(label);
            break;
        case PWQ_ERROR_INTEGER:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because of a "
                                 "bad integer value in the settings.")
                        .arg(label);
            break;
        case PWQ_ERROR_NON_INT_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because of a "
                                 "settings entry is not of integer type.")
                        .arg(label);
            break;
        case PWQ_ERROR_NON_STR_SETTING:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because of a "
                                 "settings entry is not of string type.")
                        .arg(label);
            break;
        case PWQ_ERROR_CFGFILE_OPEN:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because opening "
                                 "the configuration file failed.")
                        .arg(label);
            break;
        case PWQ_ERROR_CFGFILE_MALFORMED:
            error = c->translate("Cutelyst::ValidatorPwQuality",
                                 "Password quality check for the “%1“ field failed because the "
                                 "configuration file is malformed.")
                        .arg(label);
            break;
        case PWQ_ERROR_FATAL_FAILURE:
            error =
                c->translate(
                     "Cutelyst::ValidatorPwQuality",
                     "Password quality check for the “%1“ field failed because of a fatal failure.")
                    .arg(label);
            break;
        default:
        {
            if (returnValue < 0) {
                error = c->translate("Cutelyst::ValidatorPwQuality",
                                     "Password quality check for the “%1” field failed because of "
                                     "an unknown error.")
                            .arg(label);
            } else {
                if (returnValue < threshold) {
                    error =
                        c->translate("Cutelyst::ValidatorPwQuality",
                                     "The quality score of %1 for the password in the “%2” field "
                                     "is below the threshold of %3.")
                            .arg(QString::number(returnValue), label, QString::number(threshold));
                }
            }
        } break;
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
