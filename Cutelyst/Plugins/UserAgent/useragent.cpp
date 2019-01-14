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
#include "useragent.h"

#include <QHostAddress>
#include <QLoggingCategory>
#include <QNetworkAccessManager>

#include <QJsonDocument>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_USERAGENT, "cutelyst.useragent", QtInfoMsg)

static thread_local QNetworkAccessManager m_instance;

QNetworkAccessManager *Cutelyst::UA::networkAccessManager()
{
    return &m_instance;
}

QNetworkReply *UA::head(const QNetworkRequest &request)
{
    return m_instance.head(request);
}

QNetworkReply *UA::get(const QNetworkRequest &request)
{
    return m_instance.get(request);
}

QNetworkReply *UA::post(const QNetworkRequest &request, QIODevice *data)
{
    return m_instance.post(request, data);
}

QNetworkReply *UA::post(const QNetworkRequest &request, const QByteArray &data)
{
    return m_instance.post(request, data);
}

QNetworkReply *UA::put(const QNetworkRequest &request, QIODevice *data)
{
    return m_instance.put(request, data);
}

QNetworkReply *UA::put(const QNetworkRequest &request, const QByteArray &data)
{
    return m_instance.put(request, data);
}

QNetworkReply *UA::deleteResource(const QNetworkRequest &request)
{
    return m_instance.deleteResource(request);
}

QNetworkReply *UA::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
    return m_instance.sendCustomRequest(request, verb, data);
}

QNetworkReply *UA::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, const QByteArray &data)
{
    return m_instance.sendCustomRequest(request, verb, data);
}

QNetworkReply *UA::post(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    return m_instance.post(request, multiPart);
}

QNetworkReply *UA::put(const QNetworkRequest &request, QHttpMultiPart *multiPart)
{
    return m_instance.post(request, multiPart);
}

QNetworkReply *UA::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QHttpMultiPart *multiPart)
{
    return m_instance.sendCustomRequest(request, verb, multiPart);
}

QNetworkReply *UA::postJson(const QNetworkRequest &request, const QJsonDocument &doc)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.post(jsonRequest, doc.toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::putJson(const QNetworkRequest &request, const QJsonDocument &doc)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.put(jsonRequest, doc.toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::sendCustomRequestJson(const QNetworkRequest &request, const QByteArray &verb, const QJsonDocument &doc)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.sendCustomRequest(jsonRequest, verb, doc.toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::postJsonObject(const QNetworkRequest &request, const QJsonObject &obj)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.post(jsonRequest, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::putJsonObject(const QNetworkRequest &request, const QJsonObject &obj)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.put(jsonRequest, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::sendCustomRequestJsonObject(const QNetworkRequest &request, const QByteArray &verb, const QJsonObject &obj)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.sendCustomRequest(jsonRequest, verb, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::postJsonArray(const QNetworkRequest &request, const QJsonArray &array)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.post(jsonRequest, QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::putJsonArray(const QNetworkRequest &request, const QJsonArray &array)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.put(jsonRequest, QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::sendCustomRequestJsonArray(const QNetworkRequest &request, const QByteArray &verb, const QJsonArray &array)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/json"));
    return m_instance.sendCustomRequest(jsonRequest, verb, QJsonDocument(array).toJson(QJsonDocument::Compact));
}
