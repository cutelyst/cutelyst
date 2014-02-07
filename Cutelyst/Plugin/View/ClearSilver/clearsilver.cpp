#include "clearsilver_p.h"

#include "context.h"
#include "action.h"
#include "response.h"

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QDebug>

using namespace Cutelyst;

ClearSilver::ClearSilver(QObject *parent) :
    QObject(parent),
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
//    qDebug() << "_render" << body << data;
    return 0;
}

bool ClearSilver::render(Context *ctx)
{
    Q_D(ClearSilver);

    const QVariantHash &stash = ctx->stash();
    QString templateFile = stash.value(QLatin1String("template")).toString();
    if (templateFile.isEmpty()) {
        if (ctx->action() && !ctx->action()->privateName().isEmpty()) {
            templateFile = ctx->action()->privateName() % d->extension;
        }

        if (templateFile.isEmpty()) {
            qCritical("Cannot render template, template name or template stash key not defined");
            return false;
        }
    }

    qDebug() << "Rendering template" <<templateFile;
    QByteArray output;
    if (d->wrapper.isEmpty()) {
        if (!d->render(ctx, templateFile, stash, output)) {
            return false;
        }
    } else {
        QString wrapperFile = d->wrapper;

        QVariantHash data = stash;
        data["template"] = templateFile;

        if (!d->render(ctx, wrapperFile, data, output)) {
            return false;
        }
    }

    ctx->res()->body() = output;
    return true;
}

NEOERR* findFile(void *ctx, HDF *hdf, const char *filename, char **contents)
{
    ClearSilverPrivate *priv = static_cast<ClearSilverPrivate*>(ctx);
    if (!priv) {
        return nerr_raise(NERR_NOMEM, "Cound not cast ClearSilverPrivate");
    }

    QFile file(priv->includePath % QLatin1Char('/') % filename);

    if (!file.exists()) {
        qWarning("Cound not find file: %s", file.fileName().toLocal8Bit().data());
        return nerr_raise(NERR_NOT_FOUND, "Cound not find file: %s", file.fileName().toLocal8Bit().data());
    }

    if (!file.open(QFile::ReadOnly)) {
        qWarning("Cound not open file: %s", file.errorString().toLocal8Bit().data());
        return nerr_raise(NERR_IO, "Cound not open file: %s", file.errorString().toLocal8Bit().data());
    }

    *contents = qstrdup(file.readAll().data());
    qDebug() << "Rendering template:" << file.fileName();;
    return 0;
}

bool ClearSilverPrivate::render(Context *ctx, const QString &filename, const QVariantHash &stash, QByteArray &output)
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

void ClearSilverPrivate::renderError(Context *ctx, const QString &error)
{
    qCritical() << error;
    ctx->res()->body() = error.toUtf8();
}

HDF *ClearSilverPrivate::hdfForStash(Context *ctx, const QVariantHash &stash)
{
    HDF *hdf = 0;
    hdf_init(&hdf);

    serializeHash(hdf, stash);

    const QMetaObject *meta = ctx->metaObject();
    for (int i = 0; i < meta->propertyCount(); ++i) {
        QMetaProperty prop = meta->property(i);
        QString name = QLatin1String("ctx.") % prop.name();
        QVariant value = prop.read(ctx);
        serializeVariant(hdf, value, name);
    }
    return hdf;
}

void ClearSilverPrivate::serializeHash(HDF *hdf, const QVariantHash &hash, const QString &prefix)
{
    QString _prefix;
    if (!prefix.isNull()) {
        _prefix = prefix % QLatin1Char('.');
    }

    QVariantHash::ConstIterator it = hash.constBegin();
    while (it != hash.constEnd()) {
        serializeVariant(hdf, it.value(), _prefix % it.key());
        ++it;
    }
}

void ClearSilverPrivate::serializeMap(HDF *hdf, const QVariantMap &map, const QString &prefix)
{
    QString _prefix;
    if (!prefix.isNull()) {
        _prefix = prefix % QLatin1Char('.');
    }

    QVariantMap::ConstIterator it = map.constBegin();
    while (it != map.constEnd()) {
        serializeVariant(hdf, it.value(), _prefix % it.key());
        ++it;
    }
}

void ClearSilverPrivate::serializeVariant(HDF *hdf, const QVariant &value, const QString &key)
{
//    qDebug() << key;

    switch (value.type()) {
    case QVariant::String:
        hdf_set_value(hdf, key.toLocal8Bit().data(), value.toString().toLocal8Bit().data());
        break;
    case QVariant::Int:
        hdf_set_int_value(hdf, key.toLocal8Bit().data(), value.toInt());
        break;
    case QVariant::Hash:
        serializeHash(hdf, value.toHash(), key);
        break;
    case QVariant::Map:
        serializeMap(hdf, value.toMap(), key);
        break;
    default:
        if (value.canConvert(QVariant::String)) {
            hdf_set_value(hdf, key.toLocal8Bit().data(), value.toString().toLocal8Bit().data());
        }
        break;
    }
}
