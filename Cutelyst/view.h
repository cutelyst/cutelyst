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

#include <Cutelyst/Code>

namespace Cutelyst {

class Context;
class View : public Code
{
    Q_OBJECT
public:
    View(Application *app);
    virtual ~View();

    /**
     * The default implementation returns Code::OnlyExecute
     */
    virtual Modifiers modifiers() const;

    /**
     * All subclasses must reimplement this to
     * do it's rendering.
     * Default implementation does nothing and returns false.
     */
    virtual bool render(Context *ctx) const;

private:
    /**
     * This is used by Code execute() when
     * using an ActionView
     */
    bool doExecute(Context *ctx) { return render(ctx); }
};

}

#endif // VIEW_H
