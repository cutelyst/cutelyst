/*
 * SPDX-FileCopyrightText: (C) 2013-2023 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#pragma once

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/engine.h>

#include <QObject>

namespace Cutelyst {

class CUTELYST_LIBRARY TestEngine : public Engine
{
    Q_OBJECT
public:
    explicit TestEngine(Application *app, const QVariantMap &opts);

    int workerId() const override;

    struct TestResponse {
        QByteArray body;
        Headers headers;
        quint16 statusCode;
    };

    TestResponse createRequest(const QByteArray &method,
                               const QByteArray &path,
                               const QByteArray &query,
                               const Headers &headers,
                               QByteArray *body);

    TestResponse createRequest(const QByteArray &method,
                               const QString &path,
                               const QByteArray &query,
                               const Headers &headers,
                               QByteArray *body);

    bool init() override;

    [[nodiscard]] static QByteArray httpStatus(quint16 status);
};

} // namespace Cutelyst
