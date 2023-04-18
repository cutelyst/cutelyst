/*
 * SPDX-FileCopyrightText: (C) 2014-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef UPLOAD_H
#define UPLOAD_H

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/headers.h>

#include <QtCore/qiodevice.h>

class QTemporaryFile;

namespace Cutelyst {

class UploadPrivate;

/*! \class Upload upload.h Cutelyst/Upload
 * @brief %Cutelyst %Upload handles file upload request
 */
class CUTELYST_LIBRARY Upload final : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Upload)
public:
    /**
     * This class provides access to client upload requests
     */
    Upload(UploadPrivate *prv);
    virtual ~Upload() override;

    /**
     * Returns the name of the form field
     */
    QString name() const;

    /**
     * Returns the file name provided by the user agent
     */
    QString filename() const;

    /**
     * Returns the content type provided by the user agent
     */
    QString contentType() const;

    /**
     * Returns the headers provided by the user agent
     */
    Headers headers() const;

    /**
     * Saves this upload to the following location.
     */
    bool save(const QString &filename);

    /**
     * This function creates a temporary file and fill it with
     * the content of this upload.
     * Returns zero if an error occours.
     */
    QTemporaryFile *createTemporaryFile(const QString &templateName = QString());

    /**
     * Reimplemented from QIODevice::pos().
     */
    virtual qint64 pos() const override;

    /**
     * Reimplemented from QIODevice::size().
     */
    virtual qint64 size() const override;

    /**
     * Reimplemented from QIODevice::seek().
     */
    virtual bool seek(qint64 pos) override;

protected:
    /**
     * Reimplemented from QIODevice::readData().
     */
    virtual qint64 readData(char *data, qint64 maxlen) override;

    /**
     * Reimplemented from QIODevice::readLineData().
     */
    virtual qint64 readLineData(char *data, qint64 maxlen) override;

    /**
     * Reimplemented from QIODevice::writeData().
     */
    virtual qint64 writeData(const char *data, qint64 maxSize) override;

    UploadPrivate *d_ptr;
};

using Uploads = QVector<Upload *>;

} // namespace Cutelyst

#endif // UPLOAD_H
