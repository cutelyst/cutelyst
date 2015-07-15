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

#ifndef VIEWENGINE_H
#define VIEWENGINE_H

#include <QObject>

#include <Cutelyst/View>

namespace Cutelyst {

class Context;
class ViewInterface;
class ViewEngine : public View
{
    Q_OBJECT
public:
    explicit ViewEngine(const QString &engine, Application *app, const QString &name = QString());
    virtual ~ViewEngine();

    QStringList includePaths() const;
    void setIncludePaths(const QStringList &paths);

    Q_PROPERTY(QString templateExtension READ templateExtension WRITE setTemplateExtension)
    QString templateExtension() const;
    void setTemplateExtension(const QString &extension);

    Q_PROPERTY(QString wrapper READ wrapper WRITE setWrapper)
    QString wrapper() const;
    void setWrapper(const QString &name);

    Q_PROPERTY(bool cache READ isCaching WRITE setCache)
    bool isCaching() const;
    void setCache(bool enable);

    /**
     * Render the view using the choosen engine
     */
    virtual bool render(Context *c) const Q_DECL_OVERRIDE;

private:
    ViewInterface *m_interface;
};

}

#endif // VIEWENGINE_H
