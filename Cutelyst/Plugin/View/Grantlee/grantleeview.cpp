#include "grantleeview_p.h"

#include "context.h"
#include "action.h"
#include "response.h"

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QDebug>

using namespace Cutelyst;

GrantleeView::GrantleeView(QObject *parent) :
    QObject(parent),
    d_ptr(new GrantleeViewPrivate)
{
    Q_D(GrantleeView);
    d->loader = Grantlee::FileSystemTemplateLoader::Ptr(new Grantlee::FileSystemTemplateLoader);
    d->engine = new Grantlee::Engine(this);
    d->engine->addTemplateLoader(d->loader);
}

GrantleeView::~GrantleeView()
{
    delete d_ptr;
}

QString GrantleeView::includePath() const
{
    Q_D(const GrantleeView);
    return d->includePath;
}

void GrantleeView::setIncludePath(const QString &path)
{
    Q_D(GrantleeView);
    d->loader->setTemplateDirs(QStringList() << path);
    d->includePath = path;
}

QString GrantleeView::templateExtension() const
{
    Q_D(const GrantleeView);
    return d->extension;
}

void GrantleeView::setTemplateExtension(const QString &extension)
{
    Q_D(GrantleeView);
    d->extension = extension;
}

QString GrantleeView::wrapper() const
{
    Q_D(const GrantleeView);
    return d->wrapper;
}

void GrantleeView::setWrapper(const QString &name)
{
    Q_D(GrantleeView);
    d->wrapper = name;
}

bool GrantleeView::render(Context *ctx)
{
    Q_D(GrantleeView);

    QString templateFile = ctx->stash()[QLatin1String("template")].toString();
    if (templateFile.isEmpty()) {
        if (ctx->action() && !ctx->action()->privateName().isEmpty()) {
            templateFile = ctx->action()->privateName() % d->extension;
        }

        if (templateFile.isEmpty()) {
            qCritical("Cannot render template, template name or template stash key not defined");
            return false;
        }
    }

    Grantlee::Template tmpl;
    Grantlee::Context gCtx;
    gCtx.insert(QLatin1String("ctx"), ctx);

    const QVariantHash &stash = ctx->stash();
    QVariantHash::ConstIterator it = stash.constBegin();
    while (it != stash.constEnd()) {
        gCtx.insert(it.key(), it.value());
        ++it;
    }

    if (!d->wrapper.isEmpty()) {
        tmpl = d->engine->loadByName(d->wrapper);

        QString wrapper = tmpl->render(&gCtx);
        gCtx.insert(QLatin1String("template"), wrapper);
    }

    if (tmpl->error() == Grantlee::NoError) {
        tmpl = d->engine->loadByName(templateFile);

        QString body = tmpl->render(&gCtx);
        ctx->res()->body() = body.toUtf8();
    }

    if (tmpl->error() != Grantlee::NoError) {
        qCritical() << "Error while rendering template" << tmpl->errorString();
    }

    return tmpl->error() == Grantlee::NoError;
}
