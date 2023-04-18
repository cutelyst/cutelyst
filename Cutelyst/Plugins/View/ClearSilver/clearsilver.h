/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef CLEARSILVER_H
#define CLEARSILVER_H

#include <Cutelyst/View>

#include <QObject>
#include <QStringList>

namespace Cutelyst {

class ClearSilverPrivate;
class CUTELYST_VIEW_CLEARSILVER_EXPORT ClearSilver final : public View
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

} // namespace Cutelyst

#endif // CLEARSILVER_H
