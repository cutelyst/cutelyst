/*
 * SPDX-FileCopyrightText: (C) 2014-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_export.h>
#include <Cutelyst/headers.h>

#include <QtCore/qiodevice.h>

class QTemporaryFile;

namespace Cutelyst {

class UploadPrivate;

/**
 * \ingroup core
 * \class Upload upload.h Cutelyst/Upload
 * \brief %Cutelyst %Upload handles file upload requests.
 *
 * %Cutelyst %Upload handles file upload requests.
 */
class CUTELYST_EXPORT Upload final : public QIODevice
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Upload)
public:
    /**
     * This class provides access to client upload requests
     */
    explicit Upload(UploadPrivate *prv);
    /**
     * Destroys the %Upload object.
     */
    virtual ~Upload() override;

    /**
     * Returns the name of the form field.
     */
    [[nodiscard]] QString name() const;

    /**
     * Returns the file name provided by the user agent.
     */
    [[nodiscard]] QString filename() const;

    /**
     * Returns the content type provided by the user agent.
     */
    [[nodiscard]] QByteArray contentType() const;

    /**
     * Returns the headers provided by the user agent.
     */
    [[nodiscard]] Headers headers() const;

    /**
     * Saves this upload to the location defined by \a filename.
     */
    bool save(const QString &filename);

    /**
     * This function creates a temporary file and fill it with
     * the content of this upload.
     * Returns zero if an error occours.
     */
    [[nodiscard]] std::unique_ptr<QTemporaryFile>
        createTemporaryFile(const QString &templateName = {});

    /**
     * Reimplemented from QIODevice::pos().
     */
    qint64 pos() const override;

    /**
     * Reimplemented from QIODevice::size().
     */
    qint64 size() const override;

    /**
     * Reimplemented from QIODevice::seek().
     */
    bool seek(qint64 pos) override;

protected:
    /**
     * Reimplemented from QIODevice::readData().
     */
    qint64 readData(char *data, qint64 maxlen) override;

    /**
     * Reimplemented from QIODevice::readLineData().
     */
    qint64 readLineData(char *data, qint64 maxlen) override;

    /**
     * Reimplemented from QIODevice::writeData().
     */
    qint64 writeData(const char *data, qint64 maxSize) override;

    UploadPrivate *d_ptr;
};

using Uploads = QVector<Upload *>;

} // namespace Cutelyst
