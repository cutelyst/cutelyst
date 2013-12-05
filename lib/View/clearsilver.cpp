#include "clearsilver_p.h"

#include "cutelyst.h"
#include "cutelystaction.h"
#include "cutelystresponse.h"

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QDebug>

ClearSilver::ClearSilver(QObject *parent) :
    CutelystView(parent),
    d_ptr(new ClearSilverPrivate)
{
}

ClearSilver::~ClearSilver()
{
    delete d_ptr;
}

QString ClearSilver::includePath() const
{
    Q_D(const ClearSilver);
    return d->includePath;
}

void ClearSilver::setIncludePath(const QString &path)
{
    Q_D(ClearSilver);
    d->includePath = path;
}

QString ClearSilver::templateExtension() const
{
    Q_D(const ClearSilver);
    return d->extension;
}

void ClearSilver::setTemplateExtension(const QString &extension)
{
    Q_D(ClearSilver);
    d->extension = extension;
}

QString ClearSilver::wrapper() const
{
    Q_D(const ClearSilver);
    return d->wrapper;
}

void ClearSilver::setWrapper(const QString &name)
{
    Q_D(ClearSilver);
    d->wrapper = name;
}

NEOERR* cutelyst_render(void *user, char *data)
{
    QByteArray *body = static_cast<QByteArray*>(user);
    if (body) {
        body->append(data);
    }
//    qDebug() << "cutelyst_render" << body << data;
    return 0;
}

bool ClearSilver::render(Cutelyst *c)
{
    Q_D(ClearSilver);

    QString templateFile = c->stash()[QLatin1String("template")].toString();
    if (templateFile.isEmpty()) {
        if (c->action() && !c->action()->privateName().isEmpty()) {
            templateFile = c->action()->privateName() % d->extension;
        }

        if (templateFile.isEmpty()) {
            qCritical("Cannot render template, template name or template stash key not defined");
            return false;
        }
    }

//    if (!templateFile.startsWith(QLatin1String("/"))) {
//        templateFile = d->includePath % QLatin1Char('/') % templateFile;
//    }

    qDebug() << "Rendering template" <<templateFile;
    QByteArray output;
    if (d->wrapper.isEmpty()) {
        if (!d->render(c, templateFile, c->stash(), output)) {
            return false;
        }
    } else {
        QString wrapperFile = d->wrapper;
//        if (!wrapperFile.startsWith(QLatin1String("/"))) {
//            wrapperFile = d->includePath % QLatin1Char('/') % wrapperFile;
//        }

        QVariantHash data = c->stash();
        data["template"] = templateFile;

        if (!d->render(c, wrapperFile, data, output)) {
            return false;
        }
    }

    c->res()->body() = output;
    return true;
}

NEOERR* findFile(void *ctx, HDF *hdf, const char *filename, char **contents)
{
    qWarning() << "FIND FILE" << filename;

    ClearSilverPrivate *priv = static_cast<ClearSilverPrivate*>(ctx);
    if (!priv) {
        return nerr_raise(NERR_NOMEM, "Cound not cast ClearSilverPrivate");
    }

    QFile file(priv->includePath % QLatin1Char('/') % filename);

    if (!file.exists()) {
        return nerr_raise(NERR_NOT_FOUND, "Cound not find file: %s", file.fileName().toLocal8Bit().data());
    }

    if (!file.open(QFile::ReadOnly)) {
        return nerr_raise(NERR_NOT_FOUND, "Cound not open file: %s", file.errorString().toLocal8Bit().data());
    }

    *contents = qstrdup(file.readAll().data());
    qWarning() << "Served" << file.fileName();;
    return 0;
}

bool ClearSilverPrivate::render(Cutelyst *ctx, const QString &filename, const QVariantHash &stash, QByteArray &output)
{
    HDF *hdf = hdfForStash(ctx, stash);
    CSPARSE *cs;
    NEOERR *error;

    error = cs_init(&cs, hdf);
    if (error) {
        STRING *msg = new STRING;
        string_init(msg);
        nerr_error_traceback(error, msg);
        QString errorMsg;
        errorMsg = QString::fromLatin1("Failed to init ClearSilver:\n%1").arg(msg->buf);
        renderError(ctx, errorMsg);

        hdf_destroy(&hdf);
        nerr_ignore(&error);
        return false;
    }

    cs_register_fileload(cs, this, findFile);

    error = cs_parse_file(cs, filename.toLocal8Bit().data());
    if (error) {
        STRING *msg = new STRING;
        string_init(msg);
        nerr_error_traceback(error, msg);
        QString errorMsg;
        errorMsg = QString::fromLatin1("Failed to parse template file: %1\n%2").arg(filename, msg->buf);
        renderError(ctx, errorMsg);

        nerr_log_error(error);
        hdf_destroy(&hdf);
        nerr_ignore(&error);
        return false;
    }

    cs_render(cs, &output, cutelyst_render);

    cs_destroy(&cs);
    hdf_destroy(&hdf);

    return true;
}

void ClearSilverPrivate::renderError(Cutelyst *ctx, const QString &error)
{
    qCritical() << error;
    ctx->res()->body() = error.toUtf8();
}

HDF *ClearSilverPrivate::hdfForStash(Cutelyst *ctx, const QVariantHash &stash)
{
    HDF *hdf = 0;
    hdf_init(&hdf);

    QVariantHash::ConstIterator it = stash.constBegin();
    while (it != stash.constEnd()) {
        const QVariant &value = it.value();
        qCritical() << it.key() << it.value() << it.value().type();

        switch (value.type()) {
        case QVariant::String:
            qCritical() << it.key() << it.value();
            hdf_set_value(hdf, it.key().toLocal8Bit().data(), value.toString().toLocal8Bit().data());
            break;
        case QVariant::Int:
            hdf_set_int_value(hdf, it.key().toLocal8Bit().data(), value.toInt());
            break;
        default:
            if (value.canConvert(QVariant::String)) {
                hdf_set_value(hdf, it.key().toLocal8Bit().data(), value.toString().toLocal8Bit().data());
            }
            break;
        }
//        hdf->setValue()
        ++it;
    }

    const QMetaObject *meta = ctx->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        QMetaProperty prop = meta->property(i);
        QString name = QLatin1String("c.") % prop.name();
        QVariant value = prop.read(ctx);
        switch (prop.type()) {
        case QVariant::String:
            qCritical() << name << value;
            hdf_set_value(hdf, name.toLocal8Bit().data(), value.toString().toLocal8Bit().data());
            break;
        case QVariant::Int:
            hdf_set_int_value(hdf, name.toLocal8Bit().data(), value.toInt());
            break;
        default:
            if (value.canConvert(QVariant::String)) {
                hdf_set_value(hdf, name.toLocal8Bit().data(), value.toString().toLocal8Bit().data());
            }
            break;
        }

    }
    return hdf;
}
