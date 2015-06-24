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

#ifndef VIEWINTERFACE_H
#define VIEWINTERFACE_H

#include <Context>
#include <QString>

namespace Cutelyst {

class ViewInterface : public QObject
{
    Q_OBJECT
public:
    explicit ViewInterface(QObject *parent = 0) : QObject(parent) {}

    Q_PROPERTY(QStringList includePaths READ includePaths WRITE setIncludePaths)
    virtual QStringList includePaths() const = 0;
    virtual void setIncludePaths(const QStringList &paths) = 0;

    Q_PROPERTY(QString templateExtension READ templateExtension WRITE setTemplateExtension)
    virtual QString templateExtension() const = 0;
    virtual void setTemplateExtension(const QString &extension) = 0;

    Q_PROPERTY(QString wrapper READ wrapper WRITE setWrapper)
    virtual QString wrapper() const = 0;
    virtual void setWrapper(const QString &name) = 0;

    Q_PROPERTY(bool cache READ isCaching WRITE setCache)
    virtual bool isCaching() const = 0;
    virtual void setCache(bool enable) = 0;

    virtual bool render(Cutelyst::Context *c) = 0;
};

}

#define ViewInterface_iid "org.cutelyst.ViewInterface"

Q_DECLARE_INTERFACE(Cutelyst::ViewInterface, ViewInterface_iid)

#endif // VIEWINTERFACE_H
