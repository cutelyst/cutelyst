/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
#define CUTELYSTPLUGIN_CREDENTIALPASSWORD_H

#include <QtCore/QCryptographicHash>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/Plugins/Authentication/authentication.h>

namespace Cutelyst {

class CredentialPasswordPrivate;
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT CredentialPassword : public AuthenticationCredential
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CredentialPassword)
public:
    enum PasswordType {
        None,
        Clear,
        Hashed
    };
    Q_ENUM(PasswordType)

    /*!
     * Constructs a new CredentialPassword object with the given parent.
     */
    explicit CredentialPassword(QObject *parent = nullptr);
    virtual ~CredentialPassword();

    AuthenticationUser authenticate(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo) final;

    /*!
     * Returns the field to look for when authenticating the user. \sa authenticate().
     */
    QString passwordField() const;

    /*!
     * Sets the field to look for when authenticating the user. \sa authenticate().
     */
    void setPasswordField(const QString &fieldName);

    /*!
     * Returns the type of password this class will be dealing with.
     */
    PasswordType passwordType() const;

    /*!
     * Sets the type of password this class will be dealing with.
     */
    void setPasswordType(PasswordType type);

    /*!
     * Returns the salt string to be prepended to the password
     */
    QString passwordPreSalt() const;

    /*!
     * Sets the salt string to be prepended to the password
     */
    void setPasswordPreSalt(const QString &passwordPreSalt);

    /*!
     * Returns the salt string to be appended to the password
     */
    QString passwordPostSalt() const;

    /*!
     * Sets the salt string to be appended to the password
     */
    void setPasswordPostSalt(const QString &passwordPostSalt);

    /*!
     * Validates the given password against the correct hash.
     */
    static bool validatePassword(const QByteArray &password, const QByteArray &correctHash);

    /*!
     * Creates a password hash string.
     * \param password
     * \param method
     * \param iterations
     * \param saltByteSize
     * \param hashByteSize
     * \return the pbkdf2 representation of the password
     */
    static QByteArray createPassword(const QByteArray &password, QCryptographicHash::Algorithm method, int iterations, int saltByteSize, int hashByteSize);

    /*!
     * \brief Generates a pbkdf2 string for the given \p password
     * \param method
     * \param password
     * \param salt
     * \param rounds
     * \param keyLength
     * \return
     */
    static QByteArray pbkdf2(QCryptographicHash::Algorithm method,
                             const QByteArray &password, const QByteArray &salt,
                             int rounds, int keyLength);

    /*!
     * Generates the Hash-based message authentication code.
     */
    QByteArray hmac(QCryptographicHash::Algorithm method, QByteArray key, const QByteArray& message);

protected:
    CredentialPasswordPrivate *d_ptr;
};

} // namespace Plugin

#endif // CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
