/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
#define CUTELYSTPLUGIN_CREDENTIALPASSWORD_H

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/cutelyst_global.h>

#include <QtCore/QCryptographicHash>

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
    virtual ~CredentialPassword() override;

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
     * Validates the given password string against the correct hash string.
     */
    static bool validatePassword(const QString &password, const QString &correctHash);

    /*!
     * Creates a password hash string.
     * \note That is you want pre and post salts you must manualy add them.
     * \param password
     * \param method
     * \param iterations
     * \param saltByteSize
     * \param hashByteSize
     * \return the pbkdf2 representation of the password
     */
    static QByteArray createPassword(const QByteArray &password, QCryptographicHash::Algorithm method, int iterations, int saltByteSize, int hashByteSize);

    /*!
     * Creates a password hash string using sensible defaults
     * \note That is you want pre and post salts you must manualy add them.
     * \param password
     * \return the pbkdf2 representation of the password
     */
    static QByteArray createPassword(const QByteArray &password);

    /*!
     * Creates a password hash string using sensible defaults
     * \note That is you want pre and post salts you must manualy add them.
     * \param password
     * \return the pbkdf2 representation of the password
     */
    inline static QString createPassword(const QString &password);

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
                             const QByteArray &password,
                             const QByteArray &salt,
                             int rounds,
                             int keyLength);

    /*!
     * Generates the Hash-based message authentication code.
     */
    static QByteArray hmac(QCryptographicHash::Algorithm method, const QByteArray &key, const QByteArray &message);

protected:
    CredentialPasswordPrivate *d_ptr;
};

inline bool CredentialPassword::validatePassword(const QString &password, const QString &correctHash)
{
    return validatePassword(password.toUtf8(), correctHash.toLatin1());
}

QString CredentialPassword::createPassword(const QString &password)
{
    return QString::fromLatin1(createPassword(password.toUtf8()));
}

} // namespace Cutelyst

#endif // CUTELYSTPLUGIN_CREDENTIALPASSWORD_H
