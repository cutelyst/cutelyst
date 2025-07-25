/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "common.h"
#include "roleacl_p.h"

#include <Cutelyst/Controller>
#include <Cutelyst/Dispatcher>
#include <Cutelyst/Plugins/Authentication/authentication.h>

using namespace Qt::StringLiterals;
using namespace Cutelyst;

/**
 * \ingroup core-actions
 * \class Cutelyst::RoleACL
 * \brief User role-based authorization action role.
 *
 * Provides a reusable action role for user role-based authorization. ACLs are applied via the
 * assignment of attributes to application action subroutines.
 *
 * \code{.h}
 * class Foo : public Cutelyst::Controller
 * {
 *     Q_OBJECT
 * public:
 *     C_ATTR(foo,
 *            :Local
 *            :Does(RoleACL)
 *            :RequiresRole(admin)
 *            :ACLDetachTo(denied))
 *     void foo(Context *c);
 *
 *     C_ATTR(denied, :Local :Private :AutoArgs :ActionClass(RenderView))
 *     void denied(Context *c);
 * };
 * \endcode
 *
 * <H3>Required Attributes</H3>
 *
 * Failure to include the following required attributes will result in a fatal error when the
 * RoleACL action's constructor is called.
 *
 * \b ACLDetachTo
 *
 * The name of an action to which the request should be detached if it is determined that ACLs are
 * not satisfied for this user and the resource he is attempting to access.
 *
 * \b RequiresRole and \b AllowedRole
 *
 * The action must include at least one of these attributes, otherwise the Role::ACL constructor
 * will have a fatal error.
 *
 * <H3>Processing of ACLs</H3>
 *
 * One or more roles may be associated with an action.
 *
 * User roles are fetched via the invocation of the AuthenticationUser object’s "roles" QStringList
 * value.
 *
 * Roles specified with the RequiresRole attribute are checked before roles specified with the
 * AllowedRole attribute.
 *
 * The mandatory ACLDetachTo attribute specifies the name of the action to which execution will
 * detach on access violation.
 *
 * ACLs may be applied to chained actions so that different roles are required or allowed for each
 * link in the chain (or no roles at all).
 *
 * ACLDetachTo allows us to short-circuit traversal of an action chain as soon as access is denied
 * to one of the actions in the chain by its ACL.
 *
 * <H3>Examples</H3>
 *
 * \code{.h}
 * // this is an invalid action
 * C_ATTR(broken,
 *        :Local
 *        :Does(RoleACL))
 * void broken(Context *c);
 * \endcode
 * This action will cause a fatal error because it’s missing the ACLDetachTo attribute
 * and has neither a RequiresRole nor an AllowedRole attribute. A RoleACL action
 * must include at least one RequiresRole or AllowedRole attribute.
 *
 * \code{.h}
 * C_ATTR(foo,
 *        :Local
 *        :Does(RoleACL)
 *        :RequiresRole(admin)
 *        :ACLDetachTo(denied))
 * void foo(Context *c);
 * \endcode
 *
 * This action may only be executed by users with the 'admin' role.
 *
 * \code{.h}
 * C_ATTR(bar,
 *        :Local
 *        :Does(RoleACL)
 *        :RequiresRole(admin)
 *        :AllowedRole(editor)
 *        :AllowedRole(writer)
 *        :ACLDetachTo(denied))
 * void bar(Context *c);
 * \endcode
 *
 * This action requires that the user has the 'admin' role and either the 'editor' or 'writer' role
 * (or both).
 *
 * \code{.h}
 * C_ATTR(easy,
 *        :Local
 *        :Does(RoleACL)
 *        :AllowedRole(admin)
 *        :AllowedRole(user)
 *        :ACLDetachTo(denied))
 * void easy(Context *c);
 * \endcode
 *
 * Any user with either the 'admin' or 'user' role may execute this action.
 */
RoleACL::RoleACL(QObject *parent)
    : Component(new RoleACLPrivate, parent)
{
}

Component::Modifiers RoleACL::modifiers() const
{
    return AroundExecute;
}

bool RoleACL::init(Cutelyst::Application *application, const QVariantHash &args)
{
    Q_D(RoleACL);
    Q_UNUSED(application)

    const auto attributes = args.value(u"attributes"_s).value<ParamsMultiMap>();
    d->actionReverse      = args.value(u"reverse"_s).toString();

    if (!attributes.contains(u"RequiresRole"_s) && !attributes.contains(u"AllowedRole"_s)) {
        qFatal("RoleACL: Action %s requires at least one RequiresRole or AllowedRole attribute",
               qPrintable(d->actionReverse));
    } else {
        const QStringList required = attributes.values(u"RequiresRole"_s);
        for (const QString &role : required) {
            d->requiresRole.append(role);
        }

        const QStringList allowed = attributes.values(u"AllowedRole"_s);
        for (const QString &role : allowed) {
            d->allowedRole.append(role);
        }
    }

    auto it = attributes.constFind(u"ACLDetachTo"_s);
    if (it == attributes.constEnd() || it.value().isEmpty()) {
        qFatal("RoleACL: Action %s requires the ACLDetachTo(<action>) attribute",
               qPrintable(d->actionReverse));
    }
    d->aclDetachTo = it.value();

    return true;
}

bool RoleACL::aroundExecute(Context *c, QStack<Cutelyst::Component *> stack)
{
    Q_D(const RoleACL);

    if (canVisit(c)) {
        return Component::aroundExecute(c, stack);
    }

    c->detach(d->detachTo);

    return false;
}

bool RoleACL::canVisit(Context *c) const
{
    Q_D(const RoleACL);

    const QStringList user_has = Authentication::user(c).value(u"roles"_s).toStringList();

    const QStringList required = d->requiresRole;
    const QStringList allowed  = d->allowedRole;

    if (!required.isEmpty() && !allowed.isEmpty()) {
        bool allRequired = std::ranges::all_of(
            required, [&user_has](const QString &role) { return user_has.contains(role); });
        if (!allRequired) {
            return false;
        }

        return std::ranges::any_of(
            allowed, [&user_has](const QString &role) { return user_has.contains(role); });
    } else if (!required.isEmpty()) {
        return std::ranges::all_of(
            required, [&user_has](const QString &role) { return user_has.contains(role); });
    } else if (!allowed.isEmpty()) {
        return std::ranges::any_of(
            allowed, [&user_has](const QString &role) { return user_has.contains(role); });
    }

    return false;
}

bool RoleACL::dispatcherReady(const Dispatcher *dispatcher, Cutelyst::Controller *controller)
{
    Q_D(RoleACL);
    Q_UNUSED(dispatcher)

    d->detachTo = controller->actionFor(d->aclDetachTo);
    if (!d->detachTo) {
        d->detachTo = dispatcher->getActionByPath(d->aclDetachTo);
        if (!d->detachTo) {
            qFatal(
                "RoleACL: Action '%s' requires a valid action set on the ACLDetachTo(%s) attribute",
                qPrintable(d->actionReverse),
                qPrintable(d->aclDetachTo));
        }
    }

    return true;
}

#include "moc_roleacl.cpp"
