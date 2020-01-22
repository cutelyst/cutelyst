/*
 * Copyright (C) 2013-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef CLEARSILVER_H
#define CLEARSILVER_H

#include <QObject>
#include <QStringList>

#include <Cutelyst/View>

namespace Cutelyst {

class ClearSilverPrivate;
class CUTELYST_VIEW_CLEARSILVER_EXPORT ClearSilver : public View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClearSilver)
public:
    /*!
     * Constructs a ClearSilver object with the given parent and name.
     */
    explicit ClearSilver(QObject *parent = nullptr, const QString &name = QString());

    Q_PROPERTY(QStringList includePaths READ includePaths WRITE setIncludePaths NOTIFY changed)
    /*!
     * Returns the list of include paths
     */
    QStringList includePaths() const;

    /*!
     * Sets the list of include paths which will be looked for when resolving templates files
     */
    void setIncludePaths(const QStringList &paths);

    Q_PROPERTY(QString templateExtension READ templateExtension WRITE setTemplateExtension NOTIFY changed)
    /*!
     * Returns the template extension
     */
    QString templateExtension() const;

    /*!
     * Sets the template extension, defaults to ".html"
     */
    void setTemplateExtension(const QString &extension);

    Q_PROPERTY(QString wrapper READ wrapper WRITE setWrapper NOTIFY changed)
    /*!
     * Returns the template wrapper.
     */
    QString wrapper() const;

    /*!
     * Sets the template wrapper name, the template will be rendered into
     * content variable in which the wrapper template should render.
     */
    void setWrapper(const QString &name);

    QByteArray render(Context *c) const final;

Q_SIGNALS:
    void changed();
};

}

#endif // CLEARSILVER_H
