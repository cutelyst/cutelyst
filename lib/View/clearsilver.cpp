#include "clearsilver_p.h"

#include "cutelyst.h"
#include "cutelystaction.h"
#include "cutelystresponse.h"

#include <QString>
#include <QStringBuilder>
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

void ClearSilver::setRootDir(const QString &path)
{
    Q_D(ClearSilver);
    d->rootPath = path;
}

NEOERR* cutelyst_render(void *user, char *data)
{
    Cutelyst *c = static_cast<Cutelyst*>(user);
    if (c) {
        QByteArray body = c->res()->body();
        body.append(data);
        c->res()->setBody(body);
    }
    qWarning() << "cutelyst_render" << c << data;
    return 0;
}

bool ClearSilver::process(Cutelyst *c)
{
    Q_D(ClearSilver);

    QString templateName = c->stash()[QLatin1String("template")].toString();
    if (templateName.isEmpty()) {
        if (c->action()) {
            templateName = c->action()->privateName();
        }

        if (templateName.isEmpty()) {
            qCritical("Cannot render template, template name or template stash key not defined");
            return false;
        }
    }

    if (!templateName.startsWith(QLatin1String("/"))) {
        templateName = d->rootPath % QLatin1Char('/') % templateName;
    }

    qDebug() << templateName;
    HDF *hdf = d->hdfForStash(c->stash());

    CSPARSE *cs;
    NEOERR *error;
    error = cs_init(&cs, hdf);
    if (error) {
        qCritical("Failed to init ClearSilver: %s", error->desc);
        hdf_destroy(&hdf);
        nerr_ignore(&error);
        return false;
    }

    error = cs_parse_file(cs, templateName.toLocal8Bit().data());
    if (error) {
        STRING *msg = new STRING;
        string_init(msg);
        nerr_error_traceback(error, msg);
        QString errorMsg;
        errorMsg = QString::fromLatin1("Failed to parse template file:\n%1").arg(msg->buf);
        qCritical() << errorMsg;
        c->res()->setBody(errorMsg.toUtf8());

        nerr_log_error(error);
        hdf_destroy(&hdf);
        nerr_ignore(&error);
        return false;
    }

    cs_render(cs, c, cutelyst_render);

    cs_destroy(&cs);
    hdf_destroy(&hdf);

    return true;
}

HDF *ClearSilverPrivate::hdfForStash(const QVariantHash &stash)
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
    return hdf;
}
