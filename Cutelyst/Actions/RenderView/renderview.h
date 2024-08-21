/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <Cutelyst/action.h>
#include <Cutelyst/componentfactory.h>

namespace Cutelyst {

class RenderViewPrivate;
class RenderView final : public Action
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RenderView)
public:
    /**
     * Constructs a %RenderView object with the given \a parent.
     */
    explicit RenderView(QObject *parent = nullptr);

    /**
     * Initializes the %RenderView action by looking for a view name in the method attributes.
     * The default view will be used if no <tt>:View(name)</tt> has been declard on the
     * method attributes.
     */
    bool init(Application *application, const QVariantHash &args) override;

protected:
    /**
     * This will \link Context::forward() forward\endlink execution to either a
     * \link Context::setCutomView() custom\endlink view or to a view set as method attribute via
     * <tt>:View(name)</tt> or to the default view.
     *
     * If the \c Content-Type header of the Response has not been already set, it will be set to
     * <tt>'text/html; charset=utf-8'</tt>.
     *
     * If the Request is a HEAD request or if the Response::body() has already been set, this will
     * do nothing and will return \c true. The same is true for when the Response::status() has
     * already been set to 204 (no content) or 3xx.
     */
    bool doExecute(Cutelyst::Context *c) override;
};

class CutelystActionRenderView final
    : public QObject
    , public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    Component *createComponent(QObject *parent) override { return new RenderView(parent); }
};

} // namespace Cutelyst

#endif // RENDERVIEW_H
