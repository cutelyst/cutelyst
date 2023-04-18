/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
/*!
 * \file
 * \warning
 * This header file is part of the internal private api.
 * It is meant to be used by unit tests. See TestEngineConnection in tests/coverageobject.cpp
 * for an example on how to use it.
 */
#ifndef ENGINEREQUEST_H
#define ENGINEREQUEST_H

#include <Cutelyst/Headers>

#include <QElapsedTimer>
#include <QHostAddress>
#include <QObject>

namespace Cutelyst {

class Engine;
class Context;
class CUTELYST_LIBRARY EngineRequest
{
    Q_GADGET
    friend class Engine;

public:
    enum StatusFlag {
        InitialState     = 0x00,
        FinalizedHeaders = 0x01,
        IOWrite          = 0x02,
        Chunked          = 0x04,
        ChunkedDone      = 0x08,
        Async            = 0x10,
        Finalized        = 0x20,
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

    inline void setPath(const QString &path)
    {
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

} // namespace Cutelyst

Q_DECLARE_OPERATORS_FOR_FLAGS(Cutelyst::EngineRequest::Status)

#endif // ENGINEREQUEST_H
