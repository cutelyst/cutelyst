/*
 * Copyright (C) 2013-2014 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef GRANTLEE_VIEW_H
#define GRANTLEE_VIEW_H

#include <QObject>

#include "../ViewInterface.h"

namespace Cutelyst {

class GrantleeViewPrivate;
class GrantleeView : public ViewInterface
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GrantleeView)
    Q_PLUGIN_METADATA(IID "org.cutelyst.Grantlee" FILE "metadata.json")
    Q_INTERFACES(Cutelyst::ViewInterface)
public:
    Q_INVOKABLE explicit GrantleeView(QObject *parent = 0);
    ~GrantleeView();

    Q_PROPERTY(QStringList includePaths READ includePaths WRITE setIncludePaths)
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

    bool render(Context *c) Q_DECL_FINAL;

protected:
    GrantleeViewPrivate *d_ptr;
};

}

#endif // GRANTLEE_VIEW_H
