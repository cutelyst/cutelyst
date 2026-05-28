/*
 * SPDX-FileCopyrightText: (C) 2026 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/Plugins/Authentication/authenticationstore.h>

namespace Cutelyst {

/**
 * \ingroup plugins-authentication
 * \headerfile storeldap.h <Cutelyst/Plugins/Authentication/storeldap.h>
 * \brief Authentication store backed by an LDAP directory.
 *
 * This store is based on Catalyst::Authentication::Store::LDAP semantics:
 * user data is searched in LDAP using the configured base DN and filter,
 * then mapped to AuthenticationUser values.
 *
 * Build with OpenLDAP development files available to enable LDAP support.
 * If LDAP support is not available at build time, findUser() will always
 * return a null AuthenticationUser and log a warning.
 *
 * \logcat{plugin.authentication.ldap}
 */
class CUTELYST_PLUGIN_AUTHENTICATION_EXPORT StoreLDAP : public AuthenticationStore
{
public:
    /**
     * LDAP search scope values.
     */
    enum class SearchScope {
        Base,
        OneLevel,
        SubTree,
    };

    /**
     * Constructs a new %StoreLDAP object.
     */
    StoreLDAP();

    /**
     * Destroys the %StoreLDAP object.
     */
    ~StoreLDAP() override;

    /**
     * Reimplemented from AuthenticationStore::findUser().
     */
    AuthenticationUser findUser(Context *c, const ParamsMultiMap &userInfo) override final;

    /**
     * Reimplemented from AuthenticationStore::forSession().
     */
    QVariant forSession(Context *c, const AuthenticationUser &user) override final;

    /**
     * Reimplemented from AuthenticationStore::fromSession().
     */
    AuthenticationUser fromSession(Context *c, const QVariant &frozenUser) override final;

    /**
     * Validates clear text \a password by binding to LDAP with the user DN.
     */
    [[nodiscard]] bool validatePassword(Context *c,
                                        const AuthenticationUser &user,
                                        const QString &password) const override;

    /**
     * Sets LDAP server URI list (for example ldap://127.0.0.1:389).
     */
    void setServerUris(const QStringList &serverUris);

    /**
     * Returns LDAP server URI list.
     */
    [[nodiscard]] QStringList serverUris() const;

    /**
     * Sets bind DN used to perform LDAP searches.
     */
    void setBindDn(const QString &bindDn);

    /**
     * Returns bind DN used for LDAP searches.
     */
    [[nodiscard]] QString bindDn() const;

    /**
     * Sets bind password used to perform LDAP searches.
     */
    void setBindPassword(const QString &bindPassword);

    /**
     * Sets the LDAP base DN where users are searched.
     */
    void setUserBaseDn(const QString &baseDn);

    /**
     * Returns the LDAP base DN where users are searched.
     */
    [[nodiscard]] QString userBaseDn() const;

    /**
     * Sets the field used to get user name from auth/user info.
     */
    void setUserField(const QString &userField);

    /**
     * Returns the field used to get user name from auth/user info.
     */
    [[nodiscard]] QString userField() const;

    /**
     * Sets the LDAP attribute used as AuthenticationUser::id().
     */
    void setIdAttribute(const QString &idAttribute);

    /**
     * Returns the LDAP attribute used as AuthenticationUser::id().
     */
    [[nodiscard]] QString idAttribute() const;

    /**
     * Sets the LDAP filter template used to find a user.
     *
     * The template may contain %1 placeholder for an escaped user value.
     */
    void setUserFilter(const QString &userFilter);

    /**
     * Returns the LDAP filter template used to find a user.
     */
    [[nodiscard]] QString userFilter() const;

    /**
     * Sets LDAP search scope used to find users.
     */
    void setUserScope(SearchScope scope);

    /**
     * Returns LDAP search scope used to find users.
     */
    [[nodiscard]] SearchScope userScope() const;

    /**
     * Sets list of LDAP attributes to request. Empty list means all attributes.
     */
    void setAttributes(const QStringList &attributes);

    /**
     * Returns list of LDAP attributes requested during search.
     */
    [[nodiscard]] QStringList attributes() const;

    /**
     * Enables or disables STARTTLS before bind/search.
     */
    void setStartTls(bool startTls);

    /**
     * Returns true when STARTTLS is enabled.
     */
    [[nodiscard]] bool startTls() const;

private:
    AuthenticationUser
        findUserByAttribute(Context *c, const QString &attribute, const QString &value);

    QStringList m_serverUris;
    QString m_bindDn;
    QString m_bindPassword;
    QString m_userBaseDn;
    QString m_userField;
    QString m_idAttribute;
    QString m_userFilter;
    QStringList m_attributes;
    SearchScope m_userScope;
    bool m_startTls;
};

} // namespace Cutelyst
