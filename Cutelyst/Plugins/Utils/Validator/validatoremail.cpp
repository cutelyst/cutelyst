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

#include "validatoremail_p.h"
#include <QRegularExpression>

using namespace Cutelyst;

ValidatorEmail::ValidatorEmail(const QString &field, const QString &label, const QString &customError, QObject *parent) :
    ValidatorRule(*new ValidatorEmailPrivate(field, label, customError), parent)
{

}


ValidatorEmail::ValidatorEmail(ValidatorEmailPrivate &dd, QObject *parent) :
    ValidatorRule(dd, parent)
{

}


ValidatorEmail::~ValidatorEmail()
{

}



bool ValidatorEmail::validate()
{
    if (value().isEmpty()) {
        setValid(true);
        return true;
    }

//    bool isEmail = value().contains(QRegularExpression(QStringLiteral("^[a-zA-Z0-9.!#$%&'*+/=?^_`{|}~-]+@[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?(?:\\.[a-zA-Z0-9](?:[a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?)*$")));

    // regex taken from https://regex101.com/library/gJ7pU0
    bool isEmail = value().contains(QRegularExpression(QStringLiteral("(?(DEFINE)"
                                                                      "(?<addr_spec> (?&local_part) @ (?&domain) )"
                                                                      "(?<local_part> (?&dot_atom) | (?&quoted_string) | (?&obs_local_part) )"
                                                                      "(?<domain> (?&dot_atom) | (?&domain_literal) | (?&obs_domain) )"
                                                                      "(?<domain_literal> (?&CFWS)? \\[ (?: (?&FWS)? (?&dtext) )* (?&FWS)? \\] (?&CFWS)? )"
                                                                      "(?<dtext> [\\x21-\\x5a] | [\\x5e-\\x7e] | (?&obs_dtext) )"
                                                                      "(?<quoted_pair> \\\\ (?: (?&VCHAR) | (?&WSP) ) | (?&obs_qp) )"
                                                                      "(?<dot_atom> (?&CFWS)? (?&dot_atom_text) (?&CFWS)? )"
                                                                      "(?<dot_atom_text> (?&atext) (?: \\. (?&atext) )* )"
                                                                      "(?<atext> [a-zA-Z0-9!#$%&'*+\\/=?^_`{|}~-]+ )"
                                                                      "(?<atom> (?&CFWS)? (?&atext) (?&CFWS)? )"
                                                                      "(?<word> (?&atom) | (?&quoted_string) )"
                                                                      "(?<quoted_string> (?&CFWS)? \" (?: (?&FWS)? (?&qcontent) )* (?&FWS)? \" (?&CFWS)? )"
                                                                      "(?<qcontent> (?&qtext) | (?&quoted_pair) )"
                                                                      "(?<qtext> \\x21 | [\\x23-\\x5b] | [\\x5d-\\x7e] | (?&obs_qtext) )"
                                                                      "(?<FWS> (?: (?&WSP)* \\r\\n )? (?&WSP)+ | (?&obs_FWS) )"
                                                                      "(?<CFWS> (?: (?&FWS)? (?&comment) )+ (?&FWS)? | (?&FWS) )"
                                                                      "(?<comment> \\( (?: (?&FWS)? (?&ccontent) )* (?&FWS)? \\) )"
                                                                      "(?<ccontent> (?&ctext) | (?&quoted_pair) | (?&comment) )"
                                                                      "(?<ctext> [\\x21-\\x27] | [\\x2a-\\x5b] | [\\x5d-\\x7e] | (?&obs_ctext) )"
                                                                      "(?<obs_domain> (?&atom) (?: \\. (?&atom) )* )"
                                                                      "(?<obs_local_part> (?&word) (?: \\. (?&word) )* )"
                                                                      "(?<obs_dtext> (?&obs_NO_WS_CTL) | (?&quoted_pair) )"
                                                                      "(?<obs_qp> \\\\ (?: \\x00 | (?&obs_NO_WS_CTL) | \\n | \\r ) )"
                                                                      "(?<obs_FWS> (?&WSP)+ (?: \\r\\n (?&WSP)+ )* )"
                                                                      "(?<obs_ctext> (?&obs_NO_WS_CTL) )"
                                                                      "(?<obs_qtext> (?&obs_NO_WS_CTL) )"
                                                                      "(?<obs_NO_WS_CTL> [\\x01-\\x08] | \\x0b | \\x0c | [\\x0e-\\x1f] | \\x7f )"
                                                                      "(?<VCHAR> [\\x21-\\x7E] )"
                                                                      "(?<WSP> [ \\t] )"
                                                                  ")"
                                                                  "^(?&addr_spec)$"), QRegularExpression::ExtendedPatternSyntaxOption));

    setValid(isEmail);
    return isEmail;
}



QString ValidatorEmail::genericErrorMessage() const
{
    return tr("The email address in the “%1” field is not valid.").arg(genericFieldName());
}

