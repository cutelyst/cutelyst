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

#ifndef CLEARSILVER_H
#define CLEARSILVER_H

#include <QObject>
#include <QStringList>

#include <Cutelyst/View>

namespace Cutelyst {

class ClearSilverPrivate;
class ClearSilver : public View
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ClearSilver)
public:
    Q_INVOKABLE explicit ClearSilver(QObject *parent = 0);
    ~ClearSilver();

    Q_PROPERTY(QStringList includePaths READ includePaths WRITE setIncludePaths)
    QStringList includePaths() const;
    void setIncludePaths(const QStringList &paths);

    Q_PROPERTY(QString templateExtension READ templateExtension WRITE setTemplateExtension)
    QString templateExtension() const;
    void setTemplateExtension(const QString &extension);

    Q_PROPERTY(QString wrapper READ wrapper WRITE setWrapper)
    QString wrapper() const;
    void setWrapper(const QString &name);

    QByteArray render(Context *c) const Q_DECL_FINAL;

protected:
    ClearSilverPrivate *d_ptr;
};

}

#endif // CLEARSILVER_H
