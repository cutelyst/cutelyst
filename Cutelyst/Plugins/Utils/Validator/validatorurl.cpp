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

#include "validatorurl_p.h"
#include <QUrl>

using namespace Cutelyst;

ValidatorUrl::ValidatorUrl(const QString &field, Constraints constraints, const QStringList &schemes, const Cutelyst::ValidatorMessages &messages, const QString &defValKey) :
    ValidatorRule(*new ValidatorUrlPrivate(field, constraints, schemes, messages, defValKey))
{
}

ValidatorUrl::~ValidatorUrl()
{
}

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

        if (valid && (d->constraints.testFlag(NoRelative) || d->constraints.testFlag(WebsiteOnly)) && url.isRelative()) {
            valid = false;
        }

        if (valid && (d->constraints.testFlag(NoLocalFile) || d->constraints.testFlag(WebsiteOnly)) && url.isLocalFile()) {
            valid = false;
        }

        if (valid) {
            const QStringList schemeList = d->constraints.testFlag(WebsiteOnly) ? QStringList({QStringLiteral("http"), QStringLiteral("https")}) : d->schemes;

//            if (d->constraints.testFlag(WebsiteOnly)) {
//                if (!schemeList.contains(QStringLiteral("http"), Qt::CaseInsensitive)) {
//                    schemeList.append(QStringLiteral("http"));
//                }
//                if (!schemeList.contains(QStringLiteral("https"), Qt::CaseInsensitive)) {
//                    schemeList.append(QStringLiteral("https"));
//                }
//            }

            if (!schemeList.empty()) {

//                const QStringList sc = schemeList;
                bool foundScheme = false;
                for (const QString &s : schemeList) {
                    const QString sl =  s.toLower();
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
            qCDebug(C_VALIDATOR, "ValidatorUrl: Validation failed for field %s at %s::%s: not a valid URL", qPrintable(field()), qPrintable(c->controllerName()), qPrintable(c->actionName()));
        } else {
            result.value.setValue<QUrl>(url);
        }
    } else {
        defaultValue(c, &result, "ValidatorUrl");
    }

    return result;
}

QString ValidatorUrl::genericValidationError(Context *c, const QVariant &errorData) const
{
    QString error;
    Q_UNUSED(errorData)
    const QString _label = label(c);
    if (_label.isEmpty()) {
        error = c->translate("Cutelyst::ValidatorUrl", "Not a valid URL.");
    } else {
        error = c->translate("Cutelyst::ValidatorUrl", "The value in the “%1” field is not a valid URL.").arg(_label);
    }
    return error;
}
