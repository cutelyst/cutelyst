/*
 * Copyright (C) 2013-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef VIEW_H
#define VIEW_H

#include <QObject>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/component.h>

namespace Cutelyst {

class Context;

/*! \class View view.h Cutelyst/View
 * @brief %Cutelyst %View abstract view component
 */
class CUTELYST_LIBRARY View : public Component
{
    Q_OBJECT
public:
    /**
     * The base implementation for a View class, a name
     * for the view should be set to be found by Context->view()
     */
    explicit View(QObject *parent, const QString &name);

    /**
     * The default implementation returns Component::OnlyExecute
     */
    virtual Modifiers modifiers() const override;

    /**
     * All subclasses must reimplement this when doing it's rendering.
     * If an error (c->error()) is not set c->response()->body() is defined
     * with the returned value, this is useful if the view is not
     * meant to be used as a body.
     */
    virtual QByteArray render(Context *c) const = 0;

private:
    /**
     * This is used by Component execute() when
     * using an ActionView
     */
    bool doExecute(Context *c) final;

protected:
    /*!
     * A derived class using pimpl should call this constructor, to reduce the number of memory allocations
     */
    explicit View(ComponentPrivate *d, QObject *parent, const QString &name);
};

}

#endif // VIEW_H
