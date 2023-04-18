/*
 * SPDX-FileCopyrightText: (C) 2019-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef C_USERAGENT_H
#define C_USERAGENT_H

#include <Cutelyst/cutelyst_global.h>

#include <QNetworkReply>

class QIODevice;
class QJsonArray;
class QJsonObject;
class QJsonDocument;
class QHttpMultiPart;
class QNetworkRequest;
class QNetworkAccessManager;

namespace Cutelyst {

class Context;
class Request;

namespace UA {

CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkAccessManager *networkAccessManager();

CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *head(const QNetworkRequest &request);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *get(const QNetworkRequest &request);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *post(const QNetworkRequest &request, QIODevice *data);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *put(const QNetworkRequest &request, QIODevice *data);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *put(const QNetworkRequest &request, const QByteArray &data);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *deleteResource(const QNetworkRequest &request);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data = nullptr);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, const QByteArray &data);

CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *put(const QNetworkRequest &request, QHttpMultiPart *multiPart);
// On Qt < 5.8 it returns nullptr
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QHttpMultiPart *multiPart);

// These methods set the Content-Type header to 'application/json'
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *postJson(const QNetworkRequest &request, const QJsonDocument &doc);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *putJson(const QNetworkRequest &request, const QJsonDocument &doc);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequestJson(const QNetworkRequest &request, const QByteArray &verb, const QJsonDocument &doc);

CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *postJsonObject(const QNetworkRequest &request, const QJsonObject &obj);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *putJsonObject(const QNetworkRequest &request, const QJsonObject &obj);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequestJsonObject(const QNetworkRequest &request, const QByteArray &verb, const QJsonObject &obj);

CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *postJsonArray(const QNetworkRequest &request, const QJsonArray &obj);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *putJsonArray(const QNetworkRequest &request, const QJsonArray &obj);
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequestJsonArray(const QNetworkRequest &request, const QByteArray &verb, const QJsonArray &obj);

/**
 * This will forward your request to destination, the entire response processing should be done by your code.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *forwardRequest(Request *request, const QUrl &destination);

/**
 * This will forward your request to destination, handlying both request and response
 * but it will not call detachAsync() and attachAsync().
 *
 * Do not call deleteLater on the returned object as it will be set as the response body.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *forwardRequestResponse(Context *c, const QUrl &destination);

/**
 * This will forward your request to destination, handlying both request and response
 * and it will also call detachAsync() and attachAsync().
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT void forwardAsync(Context *c, const QUrl &destination);
} // namespace UA

} // namespace Cutelyst

#endif // C_USERAGENT_H
