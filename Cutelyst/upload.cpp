/*
 * Copyright (C) 2014-2016 Daniel Nicoletti <dantti12@gmail.com>
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

#include "upload_p.h"
#include "common.h"

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

QString Upload::contentType() const
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

    bool error = false;
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
        qint64 totalRead = 0;
        while (!atEnd()) {
            qint64 in = read(block, sizeof(block));
            if (in <= 0)
                break;
            totalRead += in;
            if (in != out.write(block, in)) {
                setErrorString(QStringLiteral("Failure to write block"));
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

QTemporaryFile *Upload::createTemporaryFile(const QString &templateName)
{
#ifndef QT_NO_TEMPORARYFILE
    Q_D(Upload);
    QTemporaryFile *ret;
    if (templateName.isEmpty()) {
        ret = new QTemporaryFile(this);
    } else {
        ret = new QTemporaryFile(templateName, this);
    }

    if (ret->open()) {
        bool error = false;
        qint64 posOrig = d->pos;
        seek(0);

        char block[4096];
        qint64 totalRead = 0;
        while (!atEnd()) {
            qint64 in = read(block, sizeof(block));
            if (in <= 0)
                break;
            totalRead += in;
            if (in != ret->write(block, in)) {
                setErrorString(QStringLiteral("Failure to write block"));
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
    delete ret;
#else
    Q_UNUSED(templateName)
#endif

    return nullptr;
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

Upload::Upload(UploadPrivate *prv) :
    d_ptr(prv)
{
    Q_D(Upload);
    open(prv->device->openMode());
    const QString disposition = prv->headers.contentDisposition();
    int start = disposition.indexOf(QLatin1String("name=\""));
    if (start != -1) {
        start += 6;
        int end = disposition.indexOf(QLatin1Char('"'), start);
        if (end != -1) {
            d->name = disposition.mid(start, end - start);
        }
    }

    start = disposition.indexOf(QLatin1String("filename=\""));
    if (start != -1) {
        start += 10;
        int end = disposition.indexOf(QLatin1Char('"'), start);
        if (end != -1) {
            d->filename = disposition.mid(start, end - start);
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
    qint64 len = d->device->read(data,
                                 qMin(size() - d->pos, maxlen));
    d->device->seek(posOrig);
    d->pos += len;
    return len;
}

qint64 Upload::readLineData(char *data, qint64 maxlen)
{
    Q_D(Upload);
    qint64 posOrig = d->device->pos();

    d->device->seek(d->startOffset + d->pos);
    qint64 len = d->device->readLine(data,
                                     qMin(size() - d->pos, maxlen));
    d->device->seek(posOrig);
    d->pos += len;
    return len;
}

qint64 Upload::writeData(const char *data, qint64 maxSize)
{
    return -1;
}

#include "moc_upload.cpp"
