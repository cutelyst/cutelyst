#include "testengine.hpp"

#include "testengine_p.h"

#include <QBuffer>

using namespace Cutelyst;

TestEngine::TestEngine(Application *app, const QVariantMap &opts)
    : Engine{app, 0, opts}
{
    QLocale::setDefault(QLocale(QLocale::English));
}

int TestEngine::workerId() const
{
    return 0;
}

TestEngine::TestResponse TestEngine::createRequest(const QByteArray &method,
                                                   const QByteArray &path,
                                                   const QByteArray &query,
                                                   const Headers &headers,
                                                   QByteArray *body)
{
    QIODevice *bodyDevice = nullptr;
    if (headers.header("Sequential"_qba).isEmpty()) {
        bodyDevice = new QBuffer(body);
    } else {
        bodyDevice = new SequentialBuffer(body);
    }
    bodyDevice->open(QIODevice::ReadOnly);

    Headers headersCL = headers;
    if (bodyDevice->size()) {
        headersCL.setContentLength(bodyDevice->size());
    }

    TestEngineConnection req;
    req.method       = method;
    QByteArray _path = path;
    req.setPath(_path);
    req.query          = query;
    req.protocol       = "HTTP/1.1"_qba;
    req.isSecure       = false;
    req.serverAddress  = "127.0.0.1"_qba;
    req.remoteAddress  = QHostAddress(u"127.0.0.1"_qs);
    req.remotePort     = 3000;
    req.remoteUser     = QString{};
    req.headers        = headersCL;
    req.startOfRequest = std::chrono::steady_clock::now();
    req.body           = bodyDevice;

    Q_EMIT processRequestAsync(&req);

    // Due async requests we create a local event loop
    req.m_eventLoop.exec();

    TestResponse ret;
    ret.body       = req.m_responseData;
    ret.statusCode = req.m_statusCode;
    ret.headers    = req.m_headers;

    return ret;
}

TestEngine::TestResponse TestEngine::createRequest(const QByteArray &method,
                                                   const QString &path,
                                                   const QByteArray &query,
                                                   const Headers &headers,
                                                   QByteArray *body)
{
    return createRequest(method, path.toLatin1(), query, headers, body);
}

bool TestEngine::init()
{
    return initApplication() && postForkApplication();
}

QByteArray TestEngine::httpStatus(quint16 status)
{
    int len;
    const auto *statusChar = httpStatusMessage(status, &len);
    return QByteArray(statusChar + 9, len - 9);
}

SequentialBuffer::SequentialBuffer(QByteArray *buffer)
    : buf(buffer)
{
}

bool SequentialBuffer::isSequential() const
{
    return true;
}

qint64 SequentialBuffer::bytesAvailable() const
{
    return buf->size() + QIODevice::bytesAvailable();
}

qint64 SequentialBuffer::readData(char *data, qint64 maxlen)
{
    QByteArray mid = buf->mid(pos(), maxlen);
    memcpy(data, mid.data(), mid.size());
    // Sequential devices consume the body
    buf->remove(0, mid.size());
    return mid.size();
}

qint64 SequentialBuffer::writeData(const char *data, qint64 len)
{
    Q_UNUSED(data);
    Q_UNUSED(len);
    return -1;
}

qint64 TestEngineConnection::doWrite(const char *data, qint64 len)
{
    m_responseData.append(data, len);
    return len;
}

bool TestEngineConnection::writeHeaders(quint16 status, const Headers &headers)
{
    m_statusCode = status;
    m_headers    = headers;

    return true;
}

void TestEngineConnection::processingFinished()
{
    m_eventLoop.quit();
}

#include "moc_testengine.cpp"
