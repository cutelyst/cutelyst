/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef ACTIONREST_H
#define ACTIONREST_H

#include <Cutelyst/action.h>
#include <Cutelyst/componentfactory.h>

namespace Cutelyst {

class ActionRESTPrivate;
class ActionREST final : public Action
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ActionREST)
public:
    /**
     * Constructs a new %ActionREST object with the given \a parent.
     */
    explicit ActionREST(QObject *parent = nullptr);

protected:
    bool doExecute(Context *c) override;
};

class CutelystActionREST final
    : public QObject
    , public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    Component *createComponent(QObject *parent) override { return new ActionREST(parent); }
};

} // namespace Cutelyst

#endif // ACTIONREST_H
