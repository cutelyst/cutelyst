/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CREDENTIALPASSWORD_P_H
#define CREDENTIALPASSWORD_P_H

#include "credentialpassword.h"

namespace Cutelyst {

class CredentialPasswordPrivate
{
public:
    bool checkPassword(const AuthenticationUser &user, const ParamsMultiMap &authinfo);
    static QByteArray cryptoEnumToStr(QCryptographicHash::Algorithm method);
    static int cryptoStrToEnum(const QByteArray &hashMethod);

    QString passwordField                         = QStringLiteral("password");
    CredentialPassword::PasswordType passwordType = CredentialPassword::None;
    QString passwordPreSalt;
    QString passwordPostSalt;
};

} // namespace Cutelyst

#endif // CREDENTIALPASSWORD_P_H
