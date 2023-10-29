/*
 * SPDX-FileCopyrightText: (C) 2019-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "useragent.h"

#include <Cutelyst/Context>
#include <Cutelyst/Engine>
#include <Cutelyst/Request>
#include <Cutelyst/Response>

#include <QBuffer>
#include <QHostAddress>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QLoggingCategory>
#include <QNetworkAccessManager>

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

QNetworkReply *
    UA::sendCustomRequest(const QNetworkRequest &request, const QByteArray &verb, QIODevice *data)
{
    return m_instance.sendCustomRequest(request, verb, data);
}

QNetworkReply *UA::sendCustomRequest(const QNetworkRequest &request,
                                     const QByteArray &verb,
                                     const QByteArray &data)
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

QNetworkReply *UA::sendCustomRequest(const QNetworkRequest &request,
                                     const QByteArray &verb,
                                     QHttpMultiPart *multiPart)
{
    return m_instance.sendCustomRequest(request, verb, multiPart);
}

QNetworkReply *UA::postJson(const QNetworkRequest &request, const QJsonDocument &doc)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return m_instance.post(jsonRequest, doc.toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::putJson(const QNetworkRequest &request, const QJsonDocument &doc)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return m_instance.put(jsonRequest, doc.toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::sendCustomRequestJson(const QNetworkRequest &request,
                                         const QByteArray &verb,
                                         const QJsonDocument &doc)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return UA::sendCustomRequest(jsonRequest, verb, doc.toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::postJsonObject(const QNetworkRequest &request, const QJsonObject &obj)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return m_instance.post(jsonRequest, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::putJsonObject(const QNetworkRequest &request, const QJsonObject &obj)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return m_instance.put(jsonRequest, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::sendCustomRequestJsonObject(const QNetworkRequest &request,
                                               const QByteArray &verb,
                                               const QJsonObject &obj)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return UA::sendCustomRequest(
        jsonRequest, verb, QJsonDocument(obj).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::postJsonArray(const QNetworkRequest &request, const QJsonArray &array)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return m_instance.post(jsonRequest, QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::putJsonArray(const QNetworkRequest &request, const QJsonArray &array)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return m_instance.put(jsonRequest, QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::sendCustomRequestJsonArray(const QNetworkRequest &request,
                                              const QByteArray &verb,
                                              const QJsonArray &array)
{
    QNetworkRequest jsonRequest(request);
    jsonRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                          QByteArrayLiteral("application/json"));
    return UA::sendCustomRequest(
        jsonRequest, verb, QJsonDocument(array).toJson(QJsonDocument::Compact));
}

QNetworkReply *UA::forwardRequest(Request *request, const QUrl &destination)
{
    QUrl dest(request->uri());
    dest.setHost(destination.host());
    dest.setPort(destination.port());
    dest.setScheme(destination.scheme());

    QNetworkRequest proxyReq(dest);

    const Headers reqHeaders = request->headers();
    const auto headersData   = reqHeaders.data();
    auto it                  = headersData.begin();
    while (it != headersData.end()) {
        proxyReq.setRawHeader(it->key, it->value);
        ++it;
    }

    return m_instance.sendCustomRequest(proxyReq, request->method(), request->body());
}

QNetworkReply *UA::forwardRequestResponse(Context *c, const QUrl &destination)
{
    QNetworkReply *reply = forwardRequest(c->request(), destination);
    QObject::connect(reply, &QNetworkReply::finished, c, [=] {
        Headers &responseHeaders                           = c->response()->headers();
        const QList<QNetworkReply::RawHeaderPair> &headers = reply->rawHeaderPairs();
        for (const QNetworkReply::RawHeaderPair &pair : headers) {
            responseHeaders.pushHeader(pair.first, pair.second);
        }
        c->response()->setStatus(
            quint16(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt()));
        c->response()->setBody(reply);
    });
    return reply;
}

void UA::forwardAsync(Context *c, const QUrl &destination)
{
    QNetworkReply *reply = forwardRequest(c->request(), destination);
    QObject::connect(reply, &QNetworkReply::finished, c, [=] {
        Headers &responseHeaders                           = c->response()->headers();
        const QList<QNetworkReply::RawHeaderPair> &headers = reply->rawHeaderPairs();
        for (const QNetworkReply::RawHeaderPair &pair : headers) {
            responseHeaders.pushHeader(pair.first, pair.second);
        }
        c->response()->setStatus(
            quint16(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toUInt()));
        c->response()->setBody(reply);
        c->attachAsync();
    });
    c->detachAsync();
}
