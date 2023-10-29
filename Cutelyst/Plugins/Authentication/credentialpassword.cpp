/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "authenticationrealm.h"
#include "credentialpassword_p.h"

#include <QFile>
#include <QLoggingCategory>
#include <QMessageAuthenticationCode>
#include <QUuid>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_CREDENTIALPASSWORD, "cutelyst.plugin.credentialpassword", QtWarningMsg)

CredentialPassword::CredentialPassword(QObject *parent)
    : AuthenticationCredential(parent)
    , d_ptr(new CredentialPasswordPrivate)
{
}

CredentialPassword::~CredentialPassword()
{
    delete d_ptr;
}

AuthenticationUser CredentialPassword::authenticate(Context *c,
                                                    AuthenticationRealm *realm,
                                                    const ParamsMultiMap &authinfo)
{
    AuthenticationUser user;
    Q_D(CredentialPassword);
    AuthenticationUser _user = realm->findUser(c, authinfo);
    if (!_user.isNull()) {
        if (d->checkPassword(_user, authinfo)) {
            user = _user;
        } else {
            qCDebug(C_CREDENTIALPASSWORD) << "Password didn't match";
        }
    } else {
        qCDebug(C_CREDENTIALPASSWORD)
            << "Unable to locate a user matching user info provided in realm";
    }
    return user;
}

QString CredentialPassword::passwordField() const
{
    Q_D(const CredentialPassword);
    return d->passwordField;
}

void CredentialPassword::setPasswordField(const QString &fieldName)
{
    Q_D(CredentialPassword);
    d->passwordField = fieldName;
}

CredentialPassword::PasswordType CredentialPassword::passwordType() const
{
    Q_D(const CredentialPassword);
    return d->passwordType;
}

void CredentialPassword::setPasswordType(Cutelyst::CredentialPassword::PasswordType type)
{
    Q_D(CredentialPassword);
    d->passwordType = type;
}

QString CredentialPassword::passwordPreSalt() const
{
    Q_D(const CredentialPassword);
    return d->passwordPreSalt;
}

void CredentialPassword::setPasswordPreSalt(const QString &passwordPreSalt)
{
    Q_D(CredentialPassword);
    d->passwordPreSalt = passwordPreSalt;
}

QString CredentialPassword::passwordPostSalt() const
{
    Q_D(const CredentialPassword);
    return d->passwordPostSalt;
}

void CredentialPassword::setPasswordPostSalt(const QString &passwordPostSalt)
{
    Q_D(CredentialPassword);
    d->passwordPostSalt = passwordPostSalt;
}

// To avoid timming attack
bool slowEquals(const QByteArray &a, const QByteArray &b)
{
    int diff = a.size() ^ b.size();
    for (int i = 0; i < a.size() && i < b.size(); i++) {
        diff |= a[i] ^ b[i];
    }
    return diff == 0;
}

#define HASH_SECTIONS 4
#define HASH_ALGORITHM_INDEX 0
#define HASH_ITERATION_INDEX 1
#define HASH_SALT_INDEX 2
#define HASH_PBKDF2_INDEX 3
bool CredentialPassword::validatePassword(const QByteArray &password, const QByteArray &correctHash)
{
    QByteArrayList params = correctHash.split(':');
    if (params.size() < HASH_SECTIONS) {
        return false;
    }

    int method = CredentialPasswordPrivate::cryptoStrToEnum(params.at(HASH_ALGORITHM_INDEX));
    if (method == -1) {
        return false;
    }

    QByteArray pbkdf2Hash = QByteArray::fromBase64(params.at(HASH_PBKDF2_INDEX));
    return slowEquals(pbkdf2Hash,
                      pbkdf2(static_cast<QCryptographicHash::Algorithm>(method),
                             password,
                             params.at(HASH_SALT_INDEX),
                             params.at(HASH_ITERATION_INDEX).toInt(),
                             pbkdf2Hash.length()));
}

QByteArray CredentialPassword::createPassword(const QByteArray &password,
                                              QCryptographicHash::Algorithm method,
                                              int iterations,
                                              int saltByteSize,
                                              int hashByteSize)
{
    QByteArray salt;
#ifdef Q_OS_LINUX
    QFile random(QStringLiteral("/dev/urandom"));
    if (random.open(QIODevice::ReadOnly)) {
        salt = random.read(saltByteSize).toBase64();
    } else {
#endif
        salt = QUuid::createUuid().toRfc4122().toBase64();
#ifdef Q_OS_LINUX
    }
#endif

    const QByteArray methodStr = CredentialPasswordPrivate::cryptoEnumToStr(method);
    return methodStr + ':' + QByteArray::number(iterations) + ':' + salt + ':' +
           pbkdf2(method, password, salt, iterations, hashByteSize).toBase64();
}

QByteArray CredentialPassword::createPassword(const QByteArray &password)
{
    return createPassword(password, QCryptographicHash::Sha512, 10000, 16, 16);
}

// TODO https://crackstation.net/hashing-security.htm
// shows a different Algorithm that seems a bit simpler
// this one does passes the RFC6070 tests
// https://www.ietf.org/rfc/rfc6070.txt
QByteArray CredentialPassword::pbkdf2(QCryptographicHash::Algorithm method,
                                      const QByteArray &password,
                                      const QByteArray &salt,
                                      int rounds,
                                      int keyLength)
{
    QByteArray key;

    if (rounds <= 0 || keyLength <= 0) {
        qCCritical(C_CREDENTIALPASSWORD, "PBKDF2 ERROR: Invalid parameters.");
        return key;
    }

    if (salt.size() == 0 || salt.size() > std::numeric_limits<int>::max() - 4) {
        return key;
    }
    key.reserve(keyLength);

    int saltSize     = salt.size();
    QByteArray asalt = salt;
    asalt.resize(saltSize + 4);

    QByteArray d1, obuf;

    QMessageAuthenticationCode code(method, password);

    for (int count = 1, remainingBytes = keyLength; remainingBytes > 0; ++count) {
        asalt[saltSize + 0] = static_cast<char>((count >> 24) & 0xff);
        asalt[saltSize + 1] = static_cast<char>((count >> 16) & 0xff);
        asalt[saltSize + 2] = static_cast<char>((count >> 8) & 0xff);
        asalt[saltSize + 3] = static_cast<char>(count & 0xff);

        code.reset();
        code.addData(asalt);
        obuf = d1 = code.result();

        for (int i = 1; i < rounds; ++i) {
            code.reset();
            code.addData(d1);
            d1        = code.result();
            auto it   = obuf.begin();
            auto d1It = d1.cbegin();
            while (d1It != d1.cend()) {
                *it = *it ^ *d1It;
                ++it;
                ++d1It;
            }
        }

        key.append(obuf);
        remainingBytes -= obuf.size();
    }

    key.truncate(keyLength);
    return key;
}

QByteArray CredentialPassword::hmac(QCryptographicHash::Algorithm method,
                                    const QByteArray &key,
                                    const QByteArray &message)
{
    return QMessageAuthenticationCode::hash(key, message, method);
}

bool CredentialPasswordPrivate::checkPassword(const AuthenticationUser &user,
                                              const ParamsMultiMap &authinfo)
{
    const QString password = passwordPreSalt + authinfo.value(passwordField) + passwordPostSalt;
    const QString storedPassword = user.value(passwordField).toString();

    if (Q_LIKELY(passwordType == CredentialPassword::Hashed)) {
        return CredentialPassword::validatePassword(password.toUtf8(), storedPassword.toUtf8());
    } else if (passwordType == CredentialPassword::Clear) {
        return storedPassword == password;
    } else if (passwordType == CredentialPassword::None) {
        qCDebug(C_CREDENTIALPASSWORD) << "CredentialPassword is set to ignore password check";
        return true;
    }

    return false;
}

QByteArray CredentialPasswordPrivate::cryptoEnumToStr(QCryptographicHash::Algorithm method)
{
    QByteArray hashmethod;

#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    if (method == QCryptographicHash::Md4) {
        hashmethod = QByteArrayLiteral("Md4");
    } else if (method == QCryptographicHash::Md5) {
        hashmethod = QByteArrayLiteral("Md5");
    }
#endif
    if (method == QCryptographicHash::Sha1) {
        hashmethod = QByteArrayLiteral("Sha1");
    }
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    if (method == QCryptographicHash::Sha224) {
        hashmethod = QByteArrayLiteral("Sha224");
    } else if (method == QCryptographicHash::Sha256) {
        hashmethod = QByteArrayLiteral("Sha256");
    } else if (method == QCryptographicHash::Sha384) {
        hashmethod = QByteArrayLiteral("Sha384");
    } else if (method == QCryptographicHash::Sha512) {
        hashmethod = QByteArrayLiteral("Sha512");
    } else if (method == QCryptographicHash::Sha3_224) {
        hashmethod = QByteArrayLiteral("Sha3_224");
    } else if (method == QCryptographicHash::Sha3_256) {
        hashmethod = QByteArrayLiteral("Sha3_256");
    } else if (method == QCryptographicHash::Sha3_384) {
        hashmethod = QByteArrayLiteral("Sha3_384");
    } else if (method == QCryptographicHash::Sha3_512) {
        hashmethod = QByteArrayLiteral("Sha3_512");
    }
#endif

    return hashmethod;
}

int CredentialPasswordPrivate::cryptoStrToEnum(const QByteArray &hashMethod)
{
    QByteArray hashmethod = hashMethod;

    int method = -1;
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    if (hashmethod == "Md4") {
        method = QCryptographicHash::Md4;
    } else if (hashmethod == "Md5") {
        method = QCryptographicHash::Md5;
    }
#endif
    if (hashmethod == "Sha1") {
        method = QCryptographicHash::Sha1;
    }
#ifndef QT_CRYPTOGRAPHICHASH_ONLY_SHA1
    if (hashmethod == "Sha224") {
        method = QCryptographicHash::Sha224;
    } else if (hashmethod == "Sha256") {
        method = QCryptographicHash::Sha256;
    } else if (hashmethod == "Sha384") {
        method = QCryptographicHash::Sha384;
    } else if (hashmethod == "Sha512") {
        method = QCryptographicHash::Sha512;
    } else if (hashmethod == "Sha3_224") {
        method = QCryptographicHash::Sha3_224;
    } else if (hashmethod == "Sha3_256") {
        method = QCryptographicHash::Sha3_256;
    } else if (hashmethod == "Sha3_384") {
        method = QCryptographicHash::Sha3_384;
    } else if (hashmethod == "Sha3_512") {
        method = QCryptographicHash::Sha3_512;
    }
#endif

    return method;
}

#include "moc_credentialpassword.cpp"
