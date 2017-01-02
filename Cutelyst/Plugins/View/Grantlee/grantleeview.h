/*
 * Copyright (C) 2013-2016 Daniel Nicoletti <dantti12@gmail.com>
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
#include <QStringList>

#include <Cutelyst/View>

namespace Grantlee {
class Engine;
}

namespace Cutelyst {

class GrantleeViewPrivate;
/**
 * GrantleeView is a Cutelyst::View handler that renders templates
 * using Grantlee engine.
 */
class CUTELYST_VIEW_GRANTLEE_EXPORT GrantleeView : public View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(GrantleeView)
public:
    explicit GrantleeView(QObject *parent = nullptr, const QString &name = QString());
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

    Grantlee::Engine *engine() const;

    void preloadTemplates();

    QByteArray render(Context *c) const final;

protected:
    GrantleeViewPrivate *d_ptr;
};

}

#endif // GRANTLEE_VIEW_H
