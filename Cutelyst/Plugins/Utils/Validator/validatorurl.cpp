/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
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

ValidatorUrl::ValidatorUrl(const QString &field, Constraints constraints, const QStringList &schemes, const QString &label, const QString &customError) :
    ValidatorRule(*new ValidatorUrlPrivate(field, constraints, schemes, label, customError))
{
}

ValidatorUrl::ValidatorUrl(ValidatorUrlPrivate &dd) :
    ValidatorRule(dd)
{
}

ValidatorUrl::~ValidatorUrl()
{
}

QString ValidatorUrl::validate() const
{
    QString result;

    Q_D(const ValidatorUrl);

    const QString v = value();

    if (!v.isEmpty()) {

        bool valid = true;

        QUrl::ParsingMode parsingMode = QUrl::TolerantMode;
        if (d->constraints.testFlag(StrictParsing)) {
            parsingMode = QUrl::StrictMode;
        }

        QUrl url(value(), parsingMode);
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
            QStringList schemeList = d->schemes;

            if (d->constraints.testFlag(WebsiteOnly)) {
                if (!schemeList.contains(QStringLiteral("http"), Qt::CaseInsensitive)) {
                    schemeList.append(QStringLiteral("http"));
                }
                if (!schemeList.contains(QStringLiteral("https"), Qt::CaseInsensitive)) {
                    schemeList.append(QStringLiteral("https"));
                }
            }

            if (!schemeList.isEmpty()) {

                const QStringList sc = schemeList;
                bool foundScheme = false;
                for (const QString &s : sc) {
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
            result = validationError();
        }
    }

    return result;
}

QString ValidatorUrl::genericValidationError() const
{
    QString error;
    if (label().isEmpty()) {
        error = QStringLiteral("Not a valid URL.");
    } else {
        error = QStringLiteral("The value in the “%1” field is not a valid URL.").arg(label());
    }
    return error;
}

void ValidatorUrl::setConstraints(Constraints constraints)
{
    Q_D(ValidatorUrl);
    d->constraints = constraints;
}

void ValidatorUrl::setSchemes(const QStringList &schemes)
{
    Q_D(ValidatorUrl);
    d->schemes = schemes;
}
