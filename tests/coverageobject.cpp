#include "coverageobject.h"
#include <QTest>
#include <QMetaObject>
#include <QString>
#include <QDebug>
#include <QLibrary>
#include <QtCore/QBuffer>

#include <Cutelyst/context.h>

#include "cutelyst_paths.h"

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

TestEngine::TestEngine(const QVariantMap &opts, QObject *parent) : Engine(opts, parent)
{

}

int TestEngine::workerId() const
{
    return 0;
}

int TestEngine::workerCore() const
{
    return 0;
}

QVariantMap TestEngine::createRequest(const QString &method, const QString &path, const QByteArray &query, const Headers &headers, QByteArray *body)
{
    QBuffer buf(body);
    buf.open(QBuffer::ReadOnly);

    QVariantMap ret;
    m_responseData = QByteArray();
    processRequest(method,
                   path,
                   query,
                   QStringLiteral("HTTP/1.1"),
                   false,
                   QStringLiteral("127.0.0.1"),
                   QHostAddress(QStringLiteral("127.0.0.1")),
                   3000,
                   QString(), // RemoteUser
                   headers,
                   QDateTime::currentMSecsSinceEpoch(),
                   &buf,
                   0);

    ret = {
        {QStringLiteral("body"), m_responseData},
        {QStringLiteral("status"), m_status},
        {QStringLiteral("headers"), QVariant::fromValue(m_headers)}
    };

    return ret;
}

bool TestEngine::finalizeHeaders(Context *ctx)
{
    Response *res = ctx->res();

    if (!Engine::finalizeHeaders(ctx)) {
        return false;
    }

    m_status = statusCode(res->status());

    m_headers = res->headers();

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
    return true;
}

#include "moc_coverageobject.cpp"
