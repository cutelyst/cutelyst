/*
 * SPDX-FileCopyrightText: (C) 2026 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "storeldap.h"

#include "common.h"

#include <QLoggingCategory>
#include <QVariantList>

#ifdef CUTELYST_PLUGIN_AUTHENTICATION_HAS_LDAP
#    include <ldap.h>
#endif

using namespace Cutelyst;
using namespace Qt::StringLiterals;

Q_LOGGING_CATEGORY(C_AUTH_LDAP, "cutelyst.plugin.authentication.ldap", QtWarningMsg)

StoreLDAP::StoreLDAP()
    : m_serverUris({u"ldap://127.0.0.1:389"_s})
    , m_userField(u"id"_s)
    , m_idAttribute(u"id"_s)
    , m_userScope(SearchScope::SubTree)
    , m_startTls(false)
{
}

StoreLDAP::~StoreLDAP()
{
}

AuthenticationUser StoreLDAP::findUser(Context *c, const ParamsMultiMap &userInfo)
{
    Q_UNUSED(c)
    if (m_userField.isEmpty()) {
        qCWarning(C_AUTH_LDAP) << "User field is empty";
        return {};
    }

    return findUserByAttribute(c, m_userField, userInfo.value(m_userField));
}

QVariant StoreLDAP::forSession(Context *c, const AuthenticationUser &user)
{
    Q_UNUSED(c)
    // Store the full user data in the session for restoration without LDAP
    return user.data();
}

AuthenticationUser StoreLDAP::fromSession(Context *c, const QVariant &frozenUser)
{
    // Try to restore from full user data if available
    if (frozenUser.canConvert<QVariantMap>()) {
        AuthenticationUser user;
        user.setData(frozenUser.toMap());
        return user;
    }
    // Fallback to old behavior (lookup by id)
    return findUserByAttribute(c, m_idAttribute, frozenUser.toString());
}

bool StoreLDAP::validatePassword(Context *c,
                                 const AuthenticationUser &user,
                                 const QString &password) const
{
    Q_UNUSED(c)

    if (password.isEmpty()) {
        return false;
    }

    const QString userDn = user.value(u"dn"_s).toString();
    if (userDn.isEmpty()) {
        qCWarning(C_AUTH_LDAP) << "LDAP self-check failed, user has no DN value";
        return false;
    }

#ifdef CUTELYST_PLUGIN_AUTHENTICATION_HAS_LDAP
    if (m_serverUris.isEmpty()) {
        qCWarning(C_AUTH_LDAP) << "No LDAP server URI configured";
        return false;
    }

    const QByteArray uri = m_serverUris.join(u" "_s).toUtf8();

    LDAP *ld = nullptr;
    int rc   = ldap_initialize(&ld, uri.constData());
    if (rc != LDAP_SUCCESS || !ld) {
        qCWarning(C_AUTH_LDAP) << "Failed to initialize LDAP connection:" << ldap_err2string(rc);
        return false;
    }

    int ldapVersion = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);

    if (m_startTls) {
        rc = ldap_start_tls_s(ld, nullptr, nullptr);
        if (rc != LDAP_SUCCESS) {
            qCWarning(C_AUTH_LDAP) << "Failed to start TLS:" << ldap_err2string(rc);
            ldap_unbind_ext_s(ld, nullptr, nullptr);
            return false;
        }
    }

    const QByteArray passwordUtf8 = password.toUtf8();
    berval bindCred;
    bindCred.bv_val = const_cast<char *>(passwordUtf8.constData());
    bindCred.bv_len = static_cast<ber_len_t>(passwordUtf8.size());

    rc = ldap_sasl_bind_s(
        ld, userDn.toUtf8().constData(), LDAP_SASL_SIMPLE, &bindCred, nullptr, nullptr, nullptr);

    if (rc == LDAP_INAPPROPRIATE_AUTH) {
        qCWarning(C_AUTH_LDAP) << "LDAP user bind failed with Inappropriate authentication. "
                                  "Server may require StartTLS/LDAPS or stronger auth."
                               << "startTls=" << m_startTls << "dn=" << userDn;
    }

    ldap_unbind_ext_s(ld, nullptr, nullptr);

    return rc == LDAP_SUCCESS;
#else
    qCWarning(C_AUTH_LDAP) << "StoreLDAP requested but plugin was built without LDAP support";
    return false;
#endif
}

void StoreLDAP::setServerUris(const QStringList &serverUris)
{
    m_serverUris = serverUris;
}

QStringList StoreLDAP::serverUris() const
{
    return m_serverUris;
}

void StoreLDAP::setBindDn(const QString &bindDn)
{
    m_bindDn = bindDn;
}

QString StoreLDAP::bindDn() const
{
    return m_bindDn;
}

void StoreLDAP::setBindPassword(const QString &bindPassword)
{
    m_bindPassword = bindPassword;
}

void StoreLDAP::setUserBaseDn(const QString &baseDn)
{
    m_userBaseDn = baseDn;
}

QString StoreLDAP::userBaseDn() const
{
    return m_userBaseDn;
}

void StoreLDAP::setUserField(const QString &userField)
{
    m_userField = userField;
}

QString StoreLDAP::userField() const
{
    return m_userField;
}

void StoreLDAP::setIdAttribute(const QString &idAttribute)
{
    m_idAttribute = idAttribute;
}

QString StoreLDAP::idAttribute() const
{
    return m_idAttribute;
}

void StoreLDAP::setUserFilter(const QString &userFilter)
{
    m_userFilter = userFilter;
}

QString StoreLDAP::userFilter() const
{
    return m_userFilter;
}

void StoreLDAP::setUserScope(StoreLDAP::SearchScope scope)
{
    m_userScope = scope;
}

StoreLDAP::SearchScope StoreLDAP::userScope() const
{
    return m_userScope;
}

void StoreLDAP::setAttributes(const QStringList &attributes)
{
    m_attributes = attributes;
}

QStringList StoreLDAP::attributes() const
{
    return m_attributes;
}

void StoreLDAP::setStartTls(bool startTls)
{
    m_startTls = startTls;
}

bool StoreLDAP::startTls() const
{
    return m_startTls;
}

#ifdef CUTELYST_PLUGIN_AUTHENTICATION_HAS_LDAP
namespace {
int toLdapScope(StoreLDAP::SearchScope scope)
{
    switch (scope) {
    case StoreLDAP::SearchScope::Base:
        return LDAP_SCOPE_BASE;
    case StoreLDAP::SearchScope::OneLevel:
        return LDAP_SCOPE_ONELEVEL;
    case StoreLDAP::SearchScope::SubTree:
    default:
        return LDAP_SCOPE_SUBTREE;
    }
}

QString escapeFilterValue(QStringView value)
{
    QString escaped;
    escaped.reserve(value.size());

    for (const QChar ch : value) {
        switch (ch.unicode()) {
        case '*':
            escaped += u"\\2a"_s;
            break;
        case '(':
            escaped += u"\\28"_s;
            break;
        case ')':
            escaped += u"\\29"_s;
            break;
        case '\\':
            escaped += u"\\5c"_s;
            break;
        case '\0':
            escaped += u"\\00"_s;
            break;
        default:
            escaped += ch;
            break;
        }
    }

    return escaped;
}

QStringList valuesToStrings(struct berval **values)
{
    QStringList out;
    if (!values) {
        return out;
    }

    for (int i = 0; values[i] != nullptr; ++i) {
        const QByteArray value(values[i]->bv_val, values[i]->bv_len);
        out.push_back(QString::fromUtf8(value));
    }

    return out;
}
} // namespace
#endif

AuthenticationUser
    StoreLDAP::findUserByAttribute(Context *c, const QString &attribute, const QString &value)
{
    Q_UNUSED(c)

    AuthenticationUser user;

    if (value.isEmpty()) {
        return user;
    }

#ifdef CUTELYST_PLUGIN_AUTHENTICATION_HAS_LDAP
    if (m_serverUris.isEmpty()) {
        qCWarning(C_AUTH_LDAP) << "No LDAP server URI configured";
        return user;
    }

    const QByteArray uri = m_serverUris.join(u" "_s).toUtf8();

    LDAP *ld = nullptr;
    int rc   = ldap_initialize(&ld, uri.constData());
    if (rc != LDAP_SUCCESS || !ld) {
        qCWarning(C_AUTH_LDAP) << "Failed to initialize LDAP connection:" << ldap_err2string(rc);
        return user;
    }

    int ldapVersion = LDAP_VERSION3;
    ldap_set_option(ld, LDAP_OPT_PROTOCOL_VERSION, &ldapVersion);

    if (m_startTls) {
        rc = ldap_start_tls_s(ld, nullptr, nullptr);
        if (rc != LDAP_SUCCESS) {
            qCWarning(C_AUTH_LDAP) << "Failed to start TLS:" << ldap_err2string(rc);
            ldap_unbind_ext_s(ld, nullptr, nullptr);
            return user;
        }
    }

    // Only bind when a service account is configured.
    // If both are empty, keep the connection anonymous and proceed with search.
    if (!m_bindDn.isEmpty() || !m_bindPassword.isEmpty()) {
        if (m_bindDn.isEmpty()) {
            qCWarning(C_AUTH_LDAP) << "LDAP bind password is set but bind DN is empty";
            ldap_unbind_ext_s(ld, nullptr, nullptr);
            return user;
        }

        const QByteArray bindPassword = m_bindPassword.toUtf8();
        const QByteArray bindDnUtf8   = m_bindDn.toUtf8();
        berval bindCred;
        bindCred.bv_val = const_cast<char *>(bindPassword.constData());
        bindCred.bv_len = static_cast<ber_len_t>(bindPassword.size());

        rc = ldap_sasl_bind_s(
            ld, bindDnUtf8.constData(), LDAP_SASL_SIMPLE, &bindCred, nullptr, nullptr, nullptr);
        if (rc != LDAP_SUCCESS) {
            if (rc == LDAP_INAPPROPRIATE_AUTH) {
                qCWarning(C_AUTH_LDAP) << "LDAP bind failed with Inappropriate authentication. "
                                          "Server may require StartTLS/LDAPS or stronger auth."
                                       << "startTls=" << m_startTls << "bindDn=" << m_bindDn;
            } else {
                qCWarning(C_AUTH_LDAP) << "LDAP bind failed:" << ldap_err2string(rc);
            }
            ldap_unbind_ext_s(ld, nullptr, nullptr);
            return user;
        }
    }

    QString filter             = m_userFilter;
    const QString escapedValue = escapeFilterValue(value);
    if (filter.isEmpty()) {
        filter = u"(%1=%2)"_s.arg(attribute, escapedValue);
    } else {
        filter = filter.arg(escapedValue);
    }

    QByteArrayList attrStorage;
    QVector<char *> attrs;
    attrs.reserve(m_attributes.size() + 1);
    for (const QString &attr : m_attributes) {
        attrStorage.push_back(attr.toUtf8());
    }
    for (QByteArray &attr : attrStorage) {
        attrs.push_back(attr.data());
    }
    attrs.push_back(nullptr);

    LDAPMessage *result = nullptr;
    rc = ldap_search_ext_s(ld,
                           m_userBaseDn.isEmpty() ? nullptr : m_userBaseDn.toUtf8().constData(),
                           toLdapScope(m_userScope),
                           filter.toUtf8().constData(),
                           m_attributes.isEmpty() ? nullptr : attrs.data(),
                           0,
                           nullptr,
                           nullptr,
                           nullptr,
                           0,
                           &result);

    if (rc != LDAP_SUCCESS) {
        qCWarning(C_AUTH_LDAP) << "LDAP search failed:" << ldap_err2string(rc);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return user;
    }

    LDAPMessage *entry = ldap_first_entry(ld, result);
    if (!entry) {
        ldap_msgfree(result);
        ldap_unbind_ext_s(ld, nullptr, nullptr);
        return user;
    }

    char *dnRaw = ldap_get_dn(ld, entry);
    if (dnRaw) {
        const QString dn = QString::fromUtf8(dnRaw);
        user.insert(u"dn"_s, dn);
        ldap_memfree(dnRaw);
    }

    BerElement *ber = nullptr;
    for (char *attr = ldap_first_attribute(ld, entry, &ber); attr != nullptr;
         attr       = ldap_next_attribute(ld, entry, ber)) {
        const QString attrName = QString::fromUtf8(attr);

        berval **values                = ldap_get_values_len(ld, entry, attr);
        const QStringList stringValues = valuesToStrings(values);

        if (stringValues.size() == 1) {
            user.insert(attrName, stringValues.constFirst());
        } else if (!stringValues.isEmpty()) {
            QVariantList list;
            list.reserve(stringValues.size());
            for (const QString &item : stringValues) {
                list.push_back(item);
            }
            user.insert(attrName, list);
        }

        if (values) {
            ldap_value_free_len(values);
        }
        ldap_memfree(attr);
    }

    if (ber) {
        ber_free(ber, 0);
    }

    const QVariant idValue =
        m_idAttribute == u"dn"_s ? user.value(u"dn"_s) : user.value(m_idAttribute);
    if (!idValue.isNull()) {
        user.setId(idValue);
    } else {
        user = AuthenticationUser();
    }

    ldap_msgfree(result);
    ldap_unbind_ext_s(ld, nullptr, nullptr);
#else
    Q_UNUSED(attribute)
    qCWarning(C_AUTH_LDAP) << "StoreLDAP requested but plugin was built without LDAP support";
#endif

    return user;
}
