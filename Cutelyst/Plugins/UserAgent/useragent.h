/*
 * Copyright (C) 2019 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#ifndef C_USERAGENT_H
#define C_USERAGENT_H

#include <Cutelyst/cutelyst_global.h>
#include <QtGlobal>

#include <QNetworkReply>

class QIODevice;
class QJsonArray;
class QJsonObject;
class QJsonDocument;
class QHttpMultiPart;
class QNetworkRequest;
class QNetworkAccessManager;

namespace Cutelyst {

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

#if QT_CONFIG(http)
    CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *post(const QNetworkRequest &request, QHttpMultiPart *multiPart);
    CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *put(const QNetworkRequest &request, QHttpMultiPart *multiPart);
    CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QHttpMultiPart *multiPart);
#endif

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
}

}

#endif // C_USERAGENT_H
