/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ROLEACL_H
#define ROLEACL_H

#include <Cutelyst/Actions/RoleACL/cutelyst_plugin_action_roleacl_export.h>
#include <Cutelyst/component.h>
#include <Cutelyst/componentfactory.h>
#include <Cutelyst/context.h>

#include <QtCore/QVariantHash>

namespace Cutelyst {

class RoleACLPrivate;
class CUTELYST_PLUGIN_ACTION_ROLEACL_EXPORT RoleACL final : public Component
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RoleACL)
public:
    /**
     * Constructs a new %RoleACL object with the given \a parent.
     */
    explicit RoleACL(QObject *parent = nullptr);

    /**
     * Always returns Component::Modifiers::AroundExecute.
     */
    Modifiers modifiers() const override;

    bool init(Application *application, const QVariantHash &args) override;

    bool aroundExecute(Context *c, QStack<Component *> stack) override;

    /**
     * Returns \c true if the action can be visited by the context \a c.
     */
    [[nodiscard]] bool canVisit(Context *c) const;

protected:
    virtual bool dispatcherReady(const Dispatcher *dispatcher, Controller *controller) override;
};

class RoleACLFactory final
    : public QObject
    , public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    virtual Component *createComponent(QObject *parent) override { return new RoleACL(parent); }
};

} // namespace Cutelyst

#endif // ROLEACL_H
