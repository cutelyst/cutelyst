#pragma once

#include <Cutelyst/enginerequest.h>

#include <QEventLoop>
#include <QIODevice>

using namespace Cutelyst;

class SequentialBuffer : public QIODevice
{
    Q_OBJECT
public:
    SequentialBuffer(QByteArray *buffer);
    bool isSequential() const override;

    qint64 bytesAvailable() const override;

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    qint64 writeData(const char *data, qint64 len) override;

private:
    QByteArray *buf;
};

class TestEngineConnection final : public Cutelyst::EngineRequest
{
public:
    TestEngineConnection() {}

protected:
    qint64 doWrite(const char *data, qint64 len) override;
    bool writeHeaders(quint16 status, const Headers &headers) override;
    void processingFinished();

public:
    QEventLoop m_eventLoop;
    QByteArray m_responseData;
    QByteArray m_status;
    Headers m_headers;
    quint16 m_statusCode;
};
