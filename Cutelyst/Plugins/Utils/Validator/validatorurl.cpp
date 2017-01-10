/*
 * Copyright (C) 2017 Matthias Fehring <kontakt@buschmann23.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "validatorurl_p.h"
#include <QUrl>

using namespace Cutelyst;

ValidatorUrl::ValidatorUrl(const QString &field, Constraints constraints, const QStringList &schemes, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorUrlPrivate(field, constraints, schemes, label, customError), parent)
{
}

ValidatorUrl::ValidatorUrl(ValidatorUrlPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{
}

ValidatorUrl::~ValidatorUrl()
{
}

bool ValidatorUrl::validate()
{
    Q_D(ValidatorUrl);

    QString v = value();

    if (v.isEmpty()) {
        setValid(true);
        return true;
    }

    QUrl::ParsingMode parsingMode = QUrl::TolerantMode;
    if (d->constraints.testFlag(StrictParsing)) {
        parsingMode = QUrl::StrictMode;
    }

    QUrl url(value(), parsingMode);
    if (!url.isValid() || url.isEmpty()) {
        return false;
    }

    if ((d->constraints.testFlag(NoRelative) || d->constraints.testFlag(WebsiteOnly)) && url.isRelative()) {
        return false;
    }

    if ((d->constraints.testFlag(NoLocalFile) || d->constraints.testFlag(WebsiteOnly)) && url.isLocalFile()) {
        return false;
    }

    if (d->constraints.testFlag(WebsiteOnly)) {
        d->schemes = QStringList({QStringLiteral("http"), QStringLiteral("https")});
    }

    if (!d->schemes.isEmpty()) {

        const QStringList sc = d->schemes;
        for (const QString &s : sc) {
            const QString sl =  s.toLower();
            if (url.scheme() == sl) {
                setValid(true);
                return true;
            }
        }

        return false;

    } else {
        setValid(true);
        return true;
    }
}

QString ValidatorUrl::genericErrorMessage() const
{
    return tr("The value in the “%1” field is not a valid URL.").arg(genericFieldName());
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
