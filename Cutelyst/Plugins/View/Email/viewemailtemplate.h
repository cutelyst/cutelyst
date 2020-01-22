/*
 * Copyright (C) 2015-2018 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef VIEWEMAILTEMPLATE_H
#define VIEWEMAILTEMPLATE_H

#include <QObject>
#include <Cutelyst/Plugins/View/Email/viewemail.h>

namespace Cutelyst {

class ViewEmailTemplatePrivate;
/**
 * ViewEmailTemplate is a Cutelyst::View handler that renders stash
 * data using another view and send the result via e-mail.
 */
class CUTELYST_VIEW_EMAIL_EXPORT ViewEmailTemplate : public ViewEmail
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ViewEmailTemplate)
    Q_PROPERTY(QString templatePrefix READ templatePrefix WRITE setTemplatePrefix NOTIFY changedProp)
    Q_PROPERTY(QString defaultView READ defaultView WRITE setDefaultView NOTIFY changedProp)
public:
    /*!
     * Constructs a new ViewEmailTemplate object with the given \p parent and \p name.
     */
    explicit ViewEmailTemplate(QObject *parent, const QString &name = QString());

    /**
     * Returns the optional prefix to look somewhere under the existing configured
     * template  paths.
     */
    QString templatePrefix() const;

    /**
     * Defines the optional prefix to look somewhere under the existing configured
     * template  paths.
     */
    void setTemplatePrefix(const QString &prefix);

    /**
     * Returns the default view used to render the templates.
     */
    QString defaultView() const;

    /**
     * Defines the default view used to render the templates.
     * If none is specified neither here nor in the stash
     * Cutelysts default view is used.
     * Warning: if you don't tell Cutelyst explicit which of your views should
     * be its default one, this class may choose the wrong one!
     */
    void setDefaultView(const QString &view);

    QByteArray render(Context *c) const override;

Q_SIGNALS:
    void changedProp();
};

}

#endif // VIEWEMAILTEMPLATE_H
