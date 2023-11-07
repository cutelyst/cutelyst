/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "application.h"
#include "componentfactory.h"
#include "context.h"
#include "renderview_p.h"
#include "response.h"
#include "view.h"

#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_RENDERVIEW, "cutelyst.renderview", QtWarningMsg)

using namespace Cutelyst;

/**
 * \class Cutelyst::RenderView renderview.h Cutelyst/Actions/RenderView/RenderView
 * \brief Sensible default end action.
 *
 * This action implements a sensible default end action, which will forward to the first available
 * view or a custom one, unless c->res()->status() is a 3xx code (redirection, not modified, etc.),
 * 204 (no content), HEAD methods, or c->res()->body() has already been set.
 *
 * If you have more than one view, you can specify which one to use with the :View(view_name)
 * attribute or one set with c->setView() otherwise this module simply calls c->view() with no
 * argument.
 *
 * The RenderView action allows to easily call a renderer without including it's
 * header and add implementation code, all that is needed is an anotation to the Controller's
 * method: \code{.h} class Users : public Cutelyst::Controller
 * {
 * public:
 *   C_ATTR(End, :ActionClass(RenderView))
 *   void End(Context *c);
 * };
 * \endcode
 * The above will render with the default
 * view added to Cutelyst::Application, if
 * you want it to render with another view
 * just add the View(name) keyword:
 * \code{.h}
 * ...
 *   C_ATTR(End, :ActionClass(RenderView) :View(ajax_view))
 *   void End(Context *c);
 * ...
 * \endcode
 */
RenderView::RenderView(QObject *parent)
    : Action(new RenderViewPrivate, parent)
{
    setObjectName(QString::fromLatin1(metaObject()->className()) + u"->execute");
}

bool RenderView::init(Cutelyst::Application *application, const QVariantHash &args)
{
    Q_D(RenderView);

    const auto attributes = args.value(u"attributes"_qs).value<ParamsMultiMap>();
    d->view               = application->view(attributes.value(u"View"_qs));

    return Action::init(application, args);
}

bool RenderView::doExecute(Cutelyst::Context *c)
{
    Q_D(const RenderView);

    if (!Action::doExecute(c)) {
        return false;
    }

    Response *res = c->res();
    if (res->contentType().isEmpty()) {
        res->setContentType("text/html; charset=utf-8"_qba);
    }

    if (c->req()->isHead()) {
        return true;
    }

    if (res->hasBody()) {
        return true;
    }

    quint16 status = res->status();
    if (status == 204 || (status >= 300 && status < 400)) {
        return true;
    }

    View *view = c->customView();
    if (view) {
        // Fist check if the user set a view
        return c->forward(view);
    } else if (d->view) {
        // Then try to use the action View attribute
        return c->forward(d->view);
    }

    qCCritical(CUTELYST_RENDERVIEW) << "Could not find a view to render.";
    res->setStatus(500);
    return false;
}

#include "moc_renderview.cpp"
