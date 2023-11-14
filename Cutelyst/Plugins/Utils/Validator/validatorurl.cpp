/*
 * SPDX-FileCopyrightText: (C) 2017-2023 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "validatorurl_p.h"

#include <QUrl>

using namespace Cutelyst;

ValidatorUrl::ValidatorUrl(const QString &field,
                           Constraints constraints,
                           const QStringList &schemes,
                           const Cutelyst::ValidatorMessages &messages,
                           const QString &defValKey)
    : ValidatorRule(*new ValidatorUrlPrivate(field, constraints, schemes, messages, defValKey))
{
}

ValidatorUrl::~ValidatorUrl() = default;

ValidatorReturnType ValidatorUrl::validate(Context *c, const ParamsMultiMap &params) const
{
    ValidatorReturnType result;

    Q_D(const ValidatorUrl);

    const QString v = value(params);

    if (!v.isEmpty()) {

        bool valid = true;

        QUrl::ParsingMode parsingMode = QUrl::TolerantMode;
        if (d->constraints.testFlag(StrictParsing)) {
            parsingMode = QUrl::StrictMode;
        }

        QUrl url(v, parsingMode);
        if (!url.isValid() || url.isEmpty()) {
            valid = false;
        }

        if (valid &&
            (d->constraints.testFlag(NoRelative) || d->constraints.testFlag(WebsiteOnly)) &&
            url.isRelative()) {
            valid = false;
        }

        if (valid &&
            (d->constraints.testFlag(NoLocalFile) || d->constraints.testFlag(WebsiteOnly)) &&
            url.isLocalFile()) {
            valid = false;
        }

        if (valid) {
            const QStringList schemeList =
                d->constraints.testFlag(WebsiteOnly)
                    ? QStringList({QStringLiteral("http"), QStringLiteral("https")})
                    : d->schemes;

            //            if (d->constraints.testFlag(WebsiteOnly)) {
            //                if (!schemeList.contains(QStringLiteral("http"), Qt::CaseInsensitive))
            //                {
            //                    schemeList.append(QStringLiteral("http"));
            //                }
            //                if (!schemeList.contains(QStringLiteral("https"),
            //                Qt::CaseInsensitive)) {
            //                    schemeList.append(QStringLiteral("https"));
            //                }
            //            }

            if (!schemeList.empty()) {

                //                const QStringList sc = schemeList;
                bool foundScheme = false;
                for (const QString &s : schemeList) {
                    const QString sl = s.toLower();
                    if (url.scheme() == sl) {
                        foundScheme = true;
                        break;
                    }
                }

                if (!foundScheme) {
                    valid = false;
                }
            }
        }

        if (!valid) {
            result.errorMessage = validationError(c);
            qCDebug(C_VALIDATOR).noquote() << debugString(c) << "Not a valid URL";
        } else {
            result.value.setValue(url);
        }
    } else {
        defaultValue(c, &result);
    }

    return result;
}

QString ValidatorUrl::genericValidationError(Context *c, const QVariant &errorData) const
{
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        //% "Not a valid URL."
        return c->qtTrId("cutelyst-valurl-genvalerr");
    } else {
        //: %1 will be replaced by the field label
        //% "The text in the “%1” field is not a valid URL."
        return c->qtTrId("cutelyst-valurl-genvalerr-label").arg(_label);
    }
}
