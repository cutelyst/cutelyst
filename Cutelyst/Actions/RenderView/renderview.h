/*
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/action.h>
#include <Cutelyst/componentfactory.h>

namespace Cutelyst {

class RenderViewPrivate;
/**
 * The RenderView action allows to easily
 * call a renderer without including it's
 * header and add implementation code, all
 * that is needed is an anotation to the
 * Controller's method:
 * \code{.h}
 * class Users : public Cutelyst::Controller
 * {
 * public:
 *   C_ATTR(End, :ActionClass(RenderView))
 *   void End(Context *c);
 * };
 * \endcode
 * The above will render with the default
 * view added to Cutelyst::Application, if
 * you want it to render with another view
 * just add the View(name) keyword:
 * \code{.h}
 * ...
 *   C_ATTR(End, :ActionClass(RenderView) :View(ajax_view))
 *   void End(Context *c);
 * ...
 * \endcode
 */
class CUTELYST_PLUGIN_ACTION_RENDERVIEW_EXPORT RenderView : public Action
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(RenderView)
public:
    explicit RenderView(QObject *parent = nullptr);
    virtual ~RenderView();

    virtual bool init(Application *application, const QVariantHash &args) Q_DECL_OVERRIDE;

protected:
    virtual bool doExecute(Cutelyst::Context *c) Q_DECL_FINAL;

    RenderViewPrivate *d_ptr;
};

class RenderViewFactory : public QObject, public ComponentFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.cutelyst.ComponentFactory" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ComponentFactory)
public:
    Component *createComponent(QObject *parent) { return new RenderView(parent); }
};

}


#endif // RENDERVIEW_H
