/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/Plugins/Authentication/authentication.h>

#include <QtCore/QCryptographicHash>

namespace Cutelyst {

class CredentialPasswordPrivate;

/**
 * \ingroup plugins-authentication
 * \headerfile credentialpassword.h <Cutelyst/Plugins/Authentication/credentialpassword.h>
 * \brief Use password based authentication to authenticate a user.
 *
 * This credential provider authenticates a user with authentication information provided
 * by for example a HTML login formular or another source for login data.
 *
 * For an example implementation see \ref plugins-authentication overview.
 *
 * \logcat{plugin.credentialpassword}
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT CredentialPassword : public AuthenticationCredential
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(CredentialPassword)
public:
    /**
     * The used password type.
     */
    enum PasswordType {
        None,  /**< Ignore password check. */
        Clear, /**< Clear text password. */
        Hashed /**< Derived password hash using
                    <A HREF="https://datatracker.ietf.org/doc/html/rfc8018">PBKDF2</A> method. */
    };
    Q_ENUM(PasswordType)

    /**
     * Constructs a new %CredentialPassword object with the given \a parent.
     */
    explicit CredentialPassword(QObject *parent = nullptr);

    /**
     * Destroys the %CredentialPassword object.
     */
    virtual ~CredentialPassword() override;

    /**
     * Tries to authenticate the user from the \a authinfo by searching it in the given \a realm.
     * If found, the password will be checked according to the set passwordType(). On success,
     * a not null AuthenticationUser object will be returned.
     */
    [[nodiscard]] AuthenticationUser
        authenticate(Context *c, AuthenticationRealm *realm, const ParamsMultiMap &authinfo) final;

    /**
     * Returns the field to look for when authenticating the user.
     * \sa authenticate(), setPasswordField()
     */
    [[nodiscard]] QString passwordField() const;

    /**
     * Sets the field to look for when authenticating the user.
     * \sa authenticate(), passwordField()
     */
    void setPasswordField(const QString &fieldName);

    /**
     * Returns the type of password this class will be dealing with.
     * \sa setPasswordType()
     */
    [[nodiscard]] PasswordType passwordType() const;

    /**
     * Sets the type of password this class will be dealing with.
     * \sa passwordType()
     */
    void setPasswordType(PasswordType type);

    /**
     * Returns the salt string to be prepended to the password.
     * \sa setPasswordPreSalt()
     */
    [[nodiscard]] QString passwordPreSalt() const;

    /**
     * Sets the salt string to be prepended to the password.
     * \sa passwordPreSalt()
     */
    void setPasswordPreSalt(const QString &passwordPreSalt);

    /**
     * Returns the salt string to be appended to the password.
     * \sa setPasswordPostSalt()
     */
    [[nodiscard]] QString passwordPostSalt() const;

    /**
     * Sets the salt string to be appended to the password.
     * \sa passwordPostSalt()
     */
    void setPasswordPostSalt(const QString &passwordPostSalt);

    /**
     * Validates the given \a password against the \a correctHash.
     */
    [[nodiscard]] static bool validatePassword(const QByteArray &password,
                                               const QByteArray &correctHash);

    /**
     * Validates the given \a password string against the \a correctHash string.
     */
    [[nodiscard]] static bool validatePassword(const QString &password, const QString &correctHash);

    /**
     * Returns a derived hash from the clear text \a password with the given \a method,
     * \a iterations, \a saltByteSize and \a hashByteSize using the pbkdf2() method.
     *
     * \note If you want to use pre and post salts you have to manually add them to the
     * \a password.
     */
    [[nodiscard]] static QByteArray createPassword(const QByteArray &password,
                                                   QCryptographicHash::Algorithm method,
                                                   int iterations,
                                                   int saltByteSize,
                                                   int hashByteSize);

    /**
     * Returns a derived hash from the clear text \a password with sensible defaults
     * using the pbkdf2() method.
     *
     * This uses SHA-512 with 10.000 iterations and 16 bytes size for salt and hash.
     *
     * \note If you want to use pre and post salts you have to manually add them to the
     * \a password.
     */
    [[nodiscard]] static QByteArray createPassword(const QByteArray &password);

    /**
     * Returns a derived hash from the clear text \a password with sensible defaults
     * using the pbkdf2() method.
     *
     * This uses SHA-512 with 10.000 iterations and 16 bytes size for salt and hash.
     *
     * \note If you want to use pre and post salts you have to manually add them to the
     * \a password.
     */
    [[nodiscard]] inline static QString createPassword(const QString &password);

    /**
     * Returns a <A HREF="https://datatracker.ietf.org/doc/html/rfc8018">PBKDF2</A> string for
     * the given clear text \a password and \a salt using \a method, \a rounds and \a keyLength.
     */
    [[nodiscard]] static QByteArray pbkdf2(QCryptographicHash::Algorithm method,
                                           const QByteArray &password,
                                           const QByteArray &salt,
                                           int rounds,
                                           int keyLength);

    /**
     * Generates the Hash-based message authentication code.
     */
    [[nodiscard]] static QByteArray hmac(QCryptographicHash::Algorithm method,
                                         const QByteArray &key,
                                         const QByteArray &message);

protected:
    CredentialPasswordPrivate *d_ptr;
};

inline bool CredentialPassword::validatePassword(const QString &password,
                                                 const QString &correctHash)
{
    return validatePassword(password.toUtf8(), correctHash.toLatin1());
}

QString CredentialPassword::createPassword(const QString &password)
{
    return QString::fromLatin1(createPassword(password.toUtf8()));
}

} // namespace Cutelyst
