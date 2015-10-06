/*
 * Copyright (C) 2013-2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef VIEW_H
#define VIEW_H

#include <QObject>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/component.h>

namespace Cutelyst {

class Context;
class CUTELYST_LIBRARY View : public Component
{
    Q_OBJECT
public:
    explicit View(QObject *parent, const QString &name = QString());
    virtual ~View();

    QString name() const;

    /**
     * The default implementation returns Component::OnlyExecute
     */
    virtual Modifiers modifiers() const Q_DECL_OVERRIDE;

    /**
     * All subclasses must reimplement this to
     * do it's rendering.
     * Default implementation does nothing and returns false.
     */
    virtual bool render(Context *c) const;

private:
    /**
     * This is used by Component execute() when
     * using an ActionView
     */
    inline bool doExecute(Context *c) Q_DECL_FINAL;
};

inline bool View::doExecute(Context *c)
{ return render(c); }

}

#endif // VIEW_H
