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

#include "clearsilver_p.h"

#include "context.h"
#include "action.h"
#include "response.h"

#include <QString>
#include <QStringBuilder>
#include <QFile>
#include <QtCore/QLoggingCategory>

Q_LOGGING_CATEGORY(CUTELYST_CLEARSILVER, "cutelyst.clearsilver")

using namespace Cutelyst;

ClearSilver::ClearSilver(QObject *parent) :
    ViewInterface(parent),
    d_ptr(new ClearSilverPrivate)
{
}

ClearSilver::~ClearSilver()
{
    delete d_ptr;
}

QStringList ClearSilver::includePaths() const
{
    Q_D(const ClearSilver);
    return d->includePaths;
}

void ClearSilver::setIncludePaths(const QStringList &paths)
{
    Q_D(ClearSilver);
    d->includePaths = paths;
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
        if (ctx->action() && !ctx->action()->reverse().isEmpty()) {
            templateFile = ctx->action()->reverse() % d->extension;
        }

        if (templateFile.isEmpty()) {
            qCCritical(CUTELYST_CLEARSILVER) << "Cannot render template, template name or template stash key not defined";
            return false;
        }
    }

    qCDebug(CUTELYST_CLEARSILVER) << "Rendering template" <<templateFile;
    QByteArray output;
    if (!d->render(ctx, templateFile, stash, output)) {
        return false;
    }

    if (!d->wrapper.isEmpty()) {
        QString wrapperFile = d->wrapper;

        QVariantHash data = stash;
        data.insert(QStringLiteral("content"), output);

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

    Q_FOREACH (const QString &includePath, priv->includePaths) {
        QFile file(includePath % QLatin1Char('/') % filename);

        if (file.exists()) {
            if (!file.open(QFile::ReadOnly)) {
                qCWarning(CUTELYST_CLEARSILVER) << "Cound not open file:" << file.errorString();
                return nerr_raise(NERR_IO, "Cound not open file: %s", file.errorString().toLocal8Bit().data());
            }

            *contents = qstrdup(file.readAll().data());
            qCDebug(CUTELYST_CLEARSILVER) << "Rendering template:" << file.fileName();;
            return 0;
        }
    }

    qCWarning(CUTELYST_CLEARSILVER) << "Cound not find file:" << filename;
    return nerr_raise(NERR_NOT_FOUND, "Cound not find file: %s", filename);
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

void ClearSilverPrivate::renderError(Context *ctx, const QString &error) const
{
    qCCritical(CUTELYST_CLEARSILVER) << error;
    ctx->res()->body() = error.toUtf8();
}

HDF *ClearSilverPrivate::hdfForStash(Context *ctx, const QVariantHash &stash) const
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

void ClearSilverPrivate::serializeHash(HDF *hdf, const QVariantHash &hash, const QString &prefix) const
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

void ClearSilverPrivate::serializeMap(HDF *hdf, const QVariantMap &map, const QString &prefix) const
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

void ClearSilverPrivate::serializeVariant(HDF *hdf, const QVariant &value, const QString &key) const
{
//    qDebug() << key;

    switch (value.type()) {
    case QMetaType::QString:
        hdf_set_value(hdf, key.toLocal8Bit().data(), value.toString().toLocal8Bit().data());
        break;
    case QMetaType::Int:
        hdf_set_int_value(hdf, key.toLocal8Bit().data(), value.toInt());
        break;
    case QMetaType::QVariantHash:
        serializeHash(hdf, value.toHash(), key);
        break;
    case QMetaType::QVariantMap:
        serializeMap(hdf, value.toMap(), key);
        break;
    default:
        if (value.canConvert(QMetaType::QString)) {
            hdf_set_value(hdf, key.toLocal8Bit().data(), value.toString().toLocal8Bit().data());
        }
        break;
    }
}


bool Cutelyst::ClearSilver::isCaching() const
{
    return false;
}

void ClearSilver::setCache(bool enable)
{
    Q_UNUSED(enable)
}
