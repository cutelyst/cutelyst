/*
 * Copyright (C) 2017 Daniel Nicoletti <dantti12@gmail.com>
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
#ifndef ENGINEREQUEST_H
#define ENGINEREQUEST_H

#include <QObject>
#include <QHostAddress>
#include <QElapsedTimer>

#include <Cutelyst/Headers>

namespace Cutelyst {

class Engine;
class Context;
class CUTELYST_LIBRARY EngineRequest
{
    Q_GADGET
    friend class Engine;
public:
    enum StatusFlag {
        InitialState = 0x00,
        FinalizedHeaders = 0x01,
        IOWrite = 0x02,
        Chunked = 0x04,
        ChunkedDone = 0x08,
        Async = 0x10,
    };
    Q_DECLARE_FLAGS(Status, StatusFlag)

    explicit EngineRequest();

    virtual ~EngineRequest();

    /*!
     * Engines must reimplement this to write the
     * response body back to the caller
     */
    virtual void finalizeBody();

    /*!
     * Engines should overwrite this if they
     * want to to make custom error messages.
     * Default implementation render an html
     * with errors.
     */
    virtual void finalizeError();

    /*!
     * Called by Application to deal
     * with finalizing cookies, headers and body
     */
    void finalize();

    /*!
     * Reimplement if you need a custom way
     * to Set-Cookie, the default implementation
     * writes them to c->res()->headers()
     */
    virtual void finalizeCookies();

    /*!
     * Finalize the headers, and call
     * doWriteHeader(), reimplemententions
     * must call this first
     */
    virtual bool finalizeHeaders();

    /*!
     * Called by Response to manually write data
     */
    qint64 write(const char *data, qint64 len);

    bool webSocketHandshake(const QString &key, const QString &origin, const QString &protocol);

    virtual bool webSocketSendTextMessage(const QString &message);

    virtual bool webSocketSendBinaryMessage(const QByteArray &message);

    virtual bool webSocketSendPing(const QByteArray &payload);

    virtual bool webSocketClose(quint16 code, const QString &reason);

protected:
    /*!
     * Reimplement this to do the RAW writing to the client
     */
    virtual qint64 doWrite(const char *data, qint64 len) = 0;

    /*!
     * This is called when the Application chain is finished
     * processing this request, here the request can send final
     * bytes to the client or do a clean up.
     *
     * Default implementation deletes both body and context.
     *
     * If a WebSocket upgrade was made then you will want to keep
     * the context object around.
     */
    virtual void processingFinished();

    /*!
     * Reimplement this to write the headers back to the client
     */
    virtual bool writeHeaders(quint16 status, const Headers &headers) = 0;

    virtual bool webSocketHandshakeDo(const QString &key, const QString &origin, const QString &protocol);

public:
    /*!
     * This method sets the path and already does the decoding so that it is
     * done a single time.
     *
     * The path requested by the user agent '/index', MUST NOT have a leading slash
     */
    void setPath(char *rawPath, const int len);

    inline void setPath(const QString &path) {
        QByteArray rawPath = path.toLatin1();
        setPath(rawPath.data(), rawPath.size());
    }

    /*! The method used (GET, POST...) */
    QString method;

    /*! Call setPath() instead */
    QString path;

    /*! The query string requested by the user agent 'foo=bar&baz' */
    QByteArray query;

    /*! The protocol requested by the user agent 'HTTP1/1' */
    QString protocol;

    /*! The server address which the server is listening to,
     *  usually the 'Host' header but if that's not present should be filled with the server address */
    QString serverAddress;

    /*! The remote/client address */
    QHostAddress remoteAddress;

    /*! The remote user name set by a front web server */
    QString remoteUser;

    /*! The request headers */
    Headers headers;

    /*! The timestamp of the start of request, TODO remove in Cutelyst 3 */
    quint64 startOfRequest = 0;

    /*! Connection status */
    Status status = InitialState;

    /*! The QIODevice containing the body (if any) of the request
     * \note It's deleted when Context gets deleted */
    QIODevice *body = nullptr;

    /*! The Cutelyst::Context of this request
     * \note It's deleted on processingFinished() or destructor */
    Context *context = nullptr;

    /*! The remote/client port */
    quint16 remotePort = 0;

    /*! If the connection is secure HTTPS */
    bool isSecure = false;

    /*! The elapsed timer since the start of request */
    QElapsedTimer elapsed;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::EngineRequest::Status)

#endif // ENGINEREQUEST_H
