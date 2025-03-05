/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "common.h"
#include "upload_p.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>

using namespace Cutelyst;

QString Upload::filename() const
{
    Q_D(const Upload);
    return d->filename;
}

QByteArray Upload::contentType() const
{
    Q_D(const Upload);
    return d->headers.contentType();
}

Headers Upload::headers() const
{
    Q_D(const Upload);
    return d->headers;
}

bool Upload::save(const QString &newName)
{
    Q_D(Upload);

    bool error           = false;
    QString fileTemplate = QStringLiteral("%1/qt_temp.XXXXXX");
    QFile out(fileTemplate.arg(QFileInfo(newName).path()));
    if (!out.open(QIODevice::ReadWrite)) {
        error = true;
    }

    if (error) {
        out.close();
        setErrorString(QLatin1String("Failed to open file for saving: ") + out.errorString());
        qCWarning(CUTELYST_UPLOAD) << errorString();
    } else {
        qint64 posOrig = d->pos;
        seek(0);

        char block[4096];
        while (!atEnd()) {
            qint64 in = read(block, sizeof(block));
            if (in <= 0) {
                break;
            }
            if (in != out.write(block, in)) {
                setErrorString(QLatin1String("Failure to write block"));
                qCWarning(CUTELYST_UPLOAD) << errorString();
                error = true;
                break;
            }
        }

        if (error) {
            out.remove();
        }

        if (!error && !out.rename(newName)) {
            error = true;
            setErrorString(QStringLiteral("Cannot create %1 for output").arg(newName));
            qCWarning(CUTELYST_UPLOAD) << errorString();
        }
        if (error) {
            out.remove();
        }
        seek(posOrig);
    }

    return !error;
}

std::unique_ptr<QTemporaryFile> Upload::createTemporaryFile(const QString &templateName)
{
    std::unique_ptr<QTemporaryFile> ret;

#ifndef QT_NO_TEMPORARYFILE
    Q_D(Upload);
    if (templateName.isEmpty()) {
        ret = std::make_unique<QTemporaryFile>();
    } else {
        ret = std::make_unique<QTemporaryFile>(templateName);
    }

    if (ret->open()) {
        bool error     = false;
        qint64 posOrig = d->pos;
        seek(0);

        char block[4096];
        while (!atEnd()) {
            qint64 in = read(block, sizeof(block));
            if (in <= 0) {
                break;
            }

            if (in != ret->write(block, in)) {
                setErrorString(QLatin1String("Failure to write block"));
                qCWarning(CUTELYST_UPLOAD) << errorString();
                error = true;
                break;
            }
        }

        if (error) {
            ret->remove();
        }
        ret->seek(0);
        seek(posOrig);

        return ret;
    } else {
        qCWarning(CUTELYST_UPLOAD) << "Failed to open temporary file.";
    }
    ret.reset();
#else
    Q_UNUSED(templateName);
#endif

    return ret;
}

qint64 Upload::pos() const
{
    Q_D(const Upload);
    return d->pos;
}

qint64 Upload::size() const
{
    Q_D(const Upload);
    return d->endOffset - d->startOffset;
}

bool Upload::seek(qint64 pos)
{
    Q_D(Upload);
    if (pos <= size()) {
        QIODevice::seek(pos);
        d->pos = pos;
        return true;
    }
    return false;
}

Upload::Upload(UploadPrivate *prv)
    : d_ptr(prv)
{
    Q_D(Upload);
    open(prv->device->openMode());
    const QByteArray disposition = prv->headers.contentDisposition();
    int start                    = disposition.indexOf("name=\"");
    if (start != -1) {
        start += 6;
        int end = disposition.indexOf(u'"', start);
        if (end != -1) {
            // TODO
            d->name = QString::fromLatin1(disposition.sliced(start, end - start));
        }
    }

    start = disposition.indexOf("filename=\"");
    if (start != -1) {
        start += 10;
        int end = disposition.indexOf('"', start);
        if (end != -1) {
            d->filename = QString::fromLatin1(disposition.sliced(start, end - start));
        }
    }
}

Upload::~Upload()
{
    delete d_ptr;
}

QString Upload::name() const
{
    Q_D(const Upload);
    return d->name;
}

qint64 Upload::readData(char *data, qint64 maxlen)
{
    Q_D(Upload);
    qint64 posOrig = d->device->pos();

    d->device->seek(d->startOffset + d->pos);
    qint64 len = d->device->read(data, qMin(size() - d->pos, maxlen));
    d->device->seek(posOrig);
    d->pos += len;
    return len;
}

qint64 Upload::readLineData(char *data, qint64 maxlen)
{
    Q_D(Upload);
    qint64 posOrig = d->device->pos();

    d->device->seek(d->startOffset + d->pos);
    qint64 len = d->device->readLine(data, qMin(size() - d->pos, maxlen));
    d->device->seek(posOrig);
    d->pos += len;
    return len;
}

qint64 Upload::writeData(const char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return -1;
}

#include "moc_upload.cpp"
