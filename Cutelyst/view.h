/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef VIEW_H
#define VIEW_H

#include <Cutelyst/component.h>
#include <Cutelyst/cutelyst_global.h>

#include <QObject>

namespace Cutelyst {

class Context;
class ViewPrivate;

/*! \class View view.h Cutelyst/View
 * @brief %Cutelyst %View abstract view component
 */
class CUTELYST_LIBRARY View : public Component
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(View)
public:
    /**
     * The base implementation for a View class, a name
     * for the view should be set to be found by Context->view()
     */
    explicit View(QObject *parent, const QString &name);
    virtual ~View() override = default;

    /**
     * The default implementation returns Component::OnlyExecute
     */
    virtual Modifiers modifiers() const override;

    /**
     * All subclasses must reimplement this when doing it's rendering.
     * If an error (c->error()) is not set c->response()->body() is defined
     * with the returned value, this is useful if the view is not
     * meant to be used as a body.
     */
    virtual QByteArray render(Context *c) const = 0;

    /**
     * Set deflate minimal size to @p minSize.
     * When @p minSize is not negative and view render output is larger than @p minSize,
     * if ACCEPT_ENCODING contains 'deflate', then deflate view render output.
     * To disable Deflate, set @p minSize to a negative integer.
     */
    void setMinimalSizeToDeflate(qint32 minSize = -1);

private:
    /**
     * This is used by Component execute() when
     * using an ActionView
     */
    bool doExecute(Context *c) final;

protected:
    /*!
     * A derived class using pimpl should call this constructor, to reduce the number of memory allocations
     */
    explicit View(ViewPrivate *d, QObject *parent, const QString &name);
};

} // namespace Cutelyst

#endif // VIEW_H
