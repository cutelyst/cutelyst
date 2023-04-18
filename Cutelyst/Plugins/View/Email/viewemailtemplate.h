/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEWEMAILTEMPLATE_H
#define VIEWEMAILTEMPLATE_H

#include <Cutelyst/Plugins/View/Email/viewemail.h>

#include <QObject>

namespace Cutelyst {

class ViewEmailTemplatePrivate;
/**
 * ViewEmailTemplate is a Cutelyst::View handler that renders stash
 * data using another view and send the result via e-mail.
 */
class CUTELYST_VIEW_EMAIL_EXPORT ViewEmailTemplate final : public ViewEmail
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

    virtual QByteArray render(Context *c) const override;

Q_SIGNALS:
    void changedProp();
};

} // namespace Cutelyst

#endif // VIEWEMAILTEMPLATE_H
