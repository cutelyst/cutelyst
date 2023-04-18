/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <Cutelyst/action.h>
#include <Cutelyst/componentfactory.h>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class RenderViewPrivate;
class CUTELYST_PLUGIN_ACTION_RENDERVIEW_EXPORT RenderView final : public Action
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RenderView)
public:
    /**
     * Constructs a RenderView object with the given \arg parent.
     */
    explicit RenderView(QObject *parent = nullptr);

    /**
     * Reimplemented from Plugin::init()
     */
    virtual bool init(Application *application, const QVariantHash &args) override;

protected:
    virtual bool doExecute(Cutelyst::Context *c) override;
};

class RenderViewFactory final : public QObject
    , public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    virtual Component *createComponent(QObject *parent) override { return new RenderView(parent); }
};

} // namespace Cutelyst

#endif // RENDERVIEW_H
