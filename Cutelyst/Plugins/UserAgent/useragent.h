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

/**
 * @ingroup plugins
 * @headerfile "" <Cutelyst/Plugins/UserAgent/UserAgent>
 * @brief Send network requests to other endpoints.
 *
 * The %UserAgent plugin provides access to a thread local network access manager object to
 * perform network requests. It also has some convenience functions for often used data types
 * like JSON.
 *
 * See also the documentation for QNetworkAccessManager to learn more about sending network
 * requests.
 *
 * \par Logging category
 * cutelyst.useragent
 */
namespace UA {

/**
 * Returns a pointer to the thread local network access manager object.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkAccessManager *networkAccessManager();

/**
 * Posts a \a request to obtain the network headers for request and returns a new QNetworkReply
 * object which will contain such headers.
 *
 * The function is named after the HTTP request associated (HEAD).
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *head(const QNetworkRequest &request);

/**
 * Posts a \a request to obtain the contents of the target request and returns a new
 * QNetworkReply object opened for reading which emits the readyRead() signal whenever
 * new data arrives.
 *
 * The contents as well as associated headers will be downloaded
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *get(const QNetworkRequest &request);

/**
 * Sends an HTTP POST \a request to the destination specified by \a request and returns
 * a new QNetworkReply object opened for reading that will contain the reply sent by the
 * server. The contents of the data device will be uploaded to the server.
 *
 * \a data must be open for reading and must remain valid until the finished() signal
 * is emitted for this reply.
 *
 * \note Sending a POST request on protocols other than HTTP and HTTPS is undefined and
 * will probably fail.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *post(const QNetworkRequest &request,
                                                     QIODevice *data);

/**
 * This is an overloaded function.
 *
 * Sends the contents of the \a data byte array to the destination specified by \a request.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *post(const QNetworkRequest &request,
                                                     const QByteArray &data);

/**
 * Uploads the contents of \a data to the destination \a request and returns a new QNetworkReply
 * object that will be open for reply.
 *
 * \a data must be opened for reading when this function is called and must remain valid until
 * the finished() signal is emitted for this reply.
 *
 * Whether anything will be available for reading from the returned object is protocol dependent.
 * For HTTP, the server may send a small HTML page indicating the upload was successful (or not).
 * Other protocols will probably have content in their replies.
 *
 * \note For HTTP, this request will send a PUT request, which most servers do not allow. Form
 * upload mechanisms, including that of uploading files through HTML forms, use the POST
 * mechanism.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *put(const QNetworkRequest &request,
                                                    QIODevice *data);

/**
 * This is an overloaded function.
 *
 * Sends the contents of the \a data byte array to the destination specified by \a request.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *put(const QNetworkRequest &request,
                                                    const QByteArray &data);

/**
 * Sends a request to delete the resource identified by the URL of \a request.
 *
 * \note This feature is currently available for HTTP only, performing an HTTP DELETE request.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *deleteResource(const QNetworkRequest &request);

/**
 * Sends a custom request to the server identified by the URL of \a request.
 *
 * It is the user's responsibility to send a \a verb to the server that is valid according to the
 * HTTP specification.
 *
 * This method provides means to send verbs other than the common ones provided via get() or post()
 * etc., for instance sending an HTTP OPTIONS command.
 *
 * If \a data is not empty, the contents of the \a data device will be uploaded to the server; in
 * that case, \a data must be open for reading and must remain valid until the finished() signal
 * is emitted for this reply.
 *
 * \note This feature is currently available for HTTP(S) only.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequest(const QNetworkRequest &request,
                                                                  const QByteArray &verb,
                                                                  QIODevice *data = nullptr);

/**
 * This is an overloaded function.
 *
 * Sends the contents of the \a data byte array to the destination specified by \a request.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequest(const QNetworkRequest &request,
                                                                  const QByteArray &verb,
                                                                  const QByteArray &data);

/**
 * This is an overloaded function.
 *
 * Sends the contents of the \a multiPart message to the destination specified by \a request.
 *
 * This can be used for sending MIME multipart messages over HTTP.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *post(const QNetworkRequest &request,
                                                     QHttpMultiPart *multiPart);

/**
 * This is an overloaded function.
 *
 * Sends the contents of the \a multiPart message to the destination specified by \a request.
 *
 * This can be used for sending MIME multipart messages over HTTP
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *put(const QNetworkRequest &request,
                                                    QHttpMultiPart *multiPart);

/**
 * This is an overloaded function.
 *
 * Sends a custom request to the server identified by the URL of \a request.
 *
 * Sends the contents of the \a multiPart message to the destination specified by \a request.
 *
 * This can be used for sending MIME multipart messages for custom verbs.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *sendCustomRequest(const QNetworkRequest &request,
                                                                  const QByteArray &verb,
                                                                  QHttpMultiPart *multiPart);

/**
 * Sends the content of the JSON document \a doc to the destination specified by \a request
 * using HTTP POST.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *postJson(const QNetworkRequest &request,
                                                         const QJsonDocument &doc);

/**
 * Sends the content of the JSON document \a doc to the destination specified by \a request
 * using HTTP PUT.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *putJson(const QNetworkRequest &request,
                                                        const QJsonDocument &doc);

/**
 * Sends the content of the JSON document \a doc to the destination specified by \a request
 * using a custom \a verb.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *
    sendCustomRequestJson(const QNetworkRequest &request,
                          const QByteArray &verb,
                          const QJsonDocument &doc);

/**
 * Sends the content of the JSON \a object to the destination specified by \a request
 * using HTTP POST.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *postJsonObject(const QNetworkRequest &request,
                                                               const QJsonObject &object);

/**
 * Sends the content of the JSON \a object obj to the destination specified by \a request
 * using HTTP PUT.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *putJsonObject(const QNetworkRequest &request,
                                                              const QJsonObject &object);

/**
 * Sends the content of the JSON \a object to the destination specified by \a request
 * using a custom \a verb.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *
    sendCustomRequestJsonObject(const QNetworkRequest &request,
                                const QByteArray &verb,
                                const QJsonObject &object);

/**
 * Sends the content of the JSON \a array to the destination specified by \a request
 * using HTTP POST.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *postJsonArray(const QNetworkRequest &request,
                                                              const QJsonArray &array);

/**
 * Sends the content of the JSON \a array to the destination specified by \a request
 * using HTTP PUT.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *putJsonArray(const QNetworkRequest &request,
                                                             const QJsonArray &array);

/**
 * Sends the content of the JSON \a array to the destination specified by \a request
 * using a custom \a verb.
 *
 * This will set the \c Content-Type header to 'application/json'.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *
    sendCustomRequestJsonArray(const QNetworkRequest &request,
                               const QByteArray &verb,
                               const QJsonArray &array);

/**
 * This will forward your \a request to \a destination, the entire response processing should
 * be done by your code.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *forwardRequest(Request *request,
                                                               const QUrl &destination);

/**
 * This will forward your \a request to \a destination, handlying both request and response
 * but it will not call detachAsync() and attachAsync().
 *
 * Do not call deleteLater on the returned object as it will be set as the response body.
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT QNetworkReply *forwardRequestResponse(Context *c,
                                                                       const QUrl &destination);

/**
 * This will forward your \a request to \a destination, handlying both request and response
 * and it will also call detachAsync() and attachAsync().
 */
CUTELYST_PLUGIN_USERAGENT_EXPORT void forwardAsync(Context *c, const QUrl &destination);
} // namespace UA

} // namespace Cutelyst

#endif // C_USERAGENT_H
