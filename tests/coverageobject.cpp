#include "coverageobject.h"
#include <QTest>
#include <QMetaObject>
#include <QString>
#include <QDebug>
#include <QLibrary>

#include <Cutelyst/context.h>

using namespace Cutelyst;

void CoverageObject::init()
{
    initTest();
}

QString CoverageObject::generateTestName() const
{
    QString test_name;
    test_name += QString::fromLatin1(metaObject()->className());
    test_name += QLatin1Char('/');
    test_name += QString::fromLatin1(QTest::currentTestFunction());
    if (QTest::currentDataTag()) {
        test_name += QLatin1Char('/');
        test_name += QString::fromLatin1(QTest::currentDataTag());
    }
    return test_name;
}

void CoverageObject::saveCoverageData()
{
#ifdef __COVERAGESCANNER__
    QString test_name;
    test_name += generateTestName();

    __coveragescanner_testname(test_name.toStdString().c_str());
    if (QTest::currentTestFailed())
        __coveragescanner_teststate("FAILED");
    else
        __coveragescanner_teststate("PASSED") ;
    __coveragescanner_save();
    __coveragescanner_testname("");
    __coveragescanner_clear();
#endif
}

void CoverageObject::cleanup()
{
    cleanupTest();
    saveCoverageData();
}

TestEngine::TestEngine(Application *app, const QVariantMap &opts) : Engine(app, 0, opts)
{

}

int TestEngine::workerId() const
{
    return 0;
}

QVariantMap TestEngine::createRequest(const QString &method, const QString &path, const QByteArray &query, const Headers &headers, QByteArray *body)
{
    QIODevice *bodyDevice = nullptr;
    if (headers.header(QStringLiteral("sequential")).isEmpty()) {
        bodyDevice = new QBuffer(body);
    } else {
        bodyDevice = new SequentialBuffer(body);
    }
    bodyDevice->open(QIODevice::ReadOnly);

    Headers headersCL = headers;
    if (bodyDevice->size()) {
        headersCL.setContentLength(bodyDevice->size());
    }

    QVariantMap ret;
    m_responseData = QByteArray();

    EngineRequest req;
    req.method = method;
    req.path = path;
    req.query = query;
    req.protocol = QStringLiteral("HTTP/1.1");
    req.isSecure = false;
    req.serverAddress = QStringLiteral("127.0.0.1");
    req.remoteAddress = QHostAddress(QStringLiteral("127.0.0.1"));
    req.remotePort = 3000;
    req.remoteUser = QString();
    req.headers = headersCL;
    req.startOfRequest = QDateTime::currentMSecsSinceEpoch();
    req.body = bodyDevice;

    processRequest(req);

    ret = {
        {QStringLiteral("body"), m_responseData},
        {QStringLiteral("status"), m_status},
        {QStringLiteral("statusCode"), m_statusCode},
        {QStringLiteral("headers"), QVariant::fromValue(m_headers)}
    };

    delete bodyDevice;

    return ret;
}

bool TestEngine::finalizeHeadersWrite(Context *c, quint16 status, const Headers &headers, void *engineData)
{
    int len;
    const auto *statusChar = httpStatusMessage(status, &len);
    m_statusCode = status;
    m_status = QByteArray(statusChar + 9, len - 9);
    m_headers = headers;

    return true;
}

qint64 TestEngine::doWrite(Context *c, const char *data, qint64 len, void *engineData)
{
    Q_UNUSED(c)
    Q_UNUSED(engineData)
    m_responseData.append(data, len);
    return len;
}

bool TestEngine::init()
{
    return initApplication() && postForkApplication();
}

SequentialBuffer::SequentialBuffer(QByteArray *buffer) : buf(buffer)
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
    return -1;
}

#include "moc_coverageobject.cpp"

