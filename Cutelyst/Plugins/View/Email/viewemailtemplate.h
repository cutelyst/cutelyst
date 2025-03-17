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
 * \ingroup plugins-view
 * \headerfile "" <Cutelyst/Plugins/View/Email/viewemailtemplate.h>
 * \brief A view that renders stash data using another view and sends it via e-mail.
 *
 * %ViewEmailTemplate is a View handler that renders Context::stash() data using another view and
 * sends it via e-mail.
 *
 * \logcat{view.emailtemplate}
 */
class CUTELYST_VIEW_EMAIL_EXPORT ViewEmailTemplate final : public ViewEmail
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ViewEmailTemplate)
    /**
     * The optional prefix to look somewhere under the existing configured template paths
     * of the used view. This will be prepended to your template file name in the form
     * <tt>prefix + / + template</tt>.
     */
    Q_PROPERTY(
        QString templatePrefix READ templatePrefix WRITE setTemplatePrefix NOTIFY changedProp)
    /**
     * The name of the default view used to render the templates.
     */
    Q_PROPERTY(QString defaultView READ defaultView WRITE setDefaultView NOTIFY changedProp)
public:
    /**
     * Constructs a new %ViewEmailTemplate object with the given \a parent and \a name.
     *
     * The \a name can be used to specify different views that can be called either dynamically
     * by Context::setCustomView() or with the \c :View() argument of the RenderView action.
     */
    explicit ViewEmailTemplate(QObject *parent, const QString &name = {});

    /**
     * Returns the optional prefix to look somewhere under the existing configured template paths
     * of the used view. This will be prepended to your template file name in the form
     * <tt>prefix + / + template</tt>.
     * \sa setTemplatePrefix()
     */
    [[nodiscard]] QString templatePrefix() const;

    /**
     * Defines the optional prefix to look somewhere under the existing configured template paths
     * of the used view. This will be prepended to your template file name in the form
     * <tt>prefix + / + template</tt>.
     * \sa templatePrefix()
     */
    void setTemplatePrefix(const QString &prefix);

    /**
     * Returns the name of the default view used to render the templates.
     * \sa setDefaultView()
     */
    [[nodiscard]] QString defaultView() const;

    /**
     * Defines the default view used to render the templates. If none is specified, neither here
     * nor in the stash, Cutelysts default view is used.
     *
     * \warning If you don’t tell %Cutelyst explicit which of your views should be
     * \ref plugins-view "its default one", this class may choose the wrong one!
     *
     * \sa defaultView()
     */
    void setDefaultView(const QString &view);

    QByteArray render(Context *c) const override;

Q_SIGNALS:
    void changedProp();
};

} // namespace Cutelyst

#endif // VIEWEMAILTEMPLATE_H
