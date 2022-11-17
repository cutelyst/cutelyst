#include "coverageobject.h"

#include <Cutelyst/Application>
#include <Cutelyst/Controller>
#include <Cutelyst/Plugins/CSRFProtection/CSRFProtection>

#include <QObject>
#include <QTest>
#include <QNetworkCookie>

using namespace Cutelyst;

class TestCsrfProtection : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestCsrfProtection(QObject *parent = nullptr) : CoverageObject(parent) {}

    void initTest();
    void cleanupTest();

private Q_SLOTS:
    void initTestCase();

    void doTest_data();
    void doTest();
    void detachToOnArgument();
    void csrfIgnorArgument();
    void ignoreNamespace();
    void ignoreNamespaceRequired();
    void csrfRedirect();

    void cleanupTestCase();

private:
    TestEngine *m_engine;
    QNetworkCookie m_cookie;
    const QString m_cookieName = QStringLiteral("xsrftoken");
    const QString m_fieldName = QStringLiteral("xsrfprotect");
    const QString m_headerName = QStringLiteral("X-MY-CSRF");
    QString m_fieldValue;

    TestEngine *getEngine();

    void performTest();
};

class CsrfprotectionTest : public Controller
{
    Q_OBJECT
public:
    explicit CsrfprotectionTest(QObject *parent) : Controller(parent) {}

    C_ATTR(testCsrf, :Local :AutoArgs)
    void testCsrf(Context *c)
    {
        c->res()->setContentType(QStringLiteral("text/plain"));
        if (c->req()->isGet()) {
            c->res()->setBody(CSRFProtection::getToken(c));
        } else {
            c->res()->setBody(QByteArrayLiteral("allowed"));
        }
    }

    C_ATTR(testCsrfIgnore, :Local :AutoArgs :CSRFIgnore)
    void testCsrfIgnore(Context *c)
    {
        c->res()->setContentType(QStringLiteral("text/plain"));
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }

    C_ATTR(testCsrfRedirect, :Local :AutoArgs)
    void testCsrfRedirect(Context *c)
    {
        c->res()->redirect(QStringLiteral("http//www.example.com"));
    }

    C_ATTR(testCsrfDetachTo, :Local :AutoArgs :CSRFDetachTo(csrfdenied))
    void testCsrfDetachTo(Context *c)
    {
        c->res()->setContentType(QStringLiteral("text/plain"));
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }

    C_ATTR(csrfdenied, :Private :AutoArgs)
    void csrfdenied(Context *c)
    {
        c->res()->setContentType(QStringLiteral("text/plain"));
        c->res()->setBody(QByteArrayLiteral("detachdenied"));
        c->detach();
    }
};

class CsrfprotectionNsTest : public Controller
{
    Q_OBJECT
    C_NAMESPACE("testns")
public:
    explicit CsrfprotectionNsTest(QObject *parent) : Controller(parent) {}

    C_ATTR(testCsrf, :Local :AutoArgs)
    void testCsrf(Context *c)
    {
        c->res()->setContentType(QStringLiteral("text/plain"));
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }

    C_ATTR(testCsrfRequired, :Local :AutoArgs :CSRFRequire)
    void testCsrfRequired(Context *c)
    {
        c->res()->setContentType(QStringLiteral("text/plain"));
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }
};

void TestCsrfProtection::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
    if (m_cookie.value().isEmpty()) {
        const QVariantMap result = m_engine->createRequest(QStringLiteral("GET"), QStringLiteral("csrfprotection/test/testCsrf"), QByteArray(), Headers(), nullptr);
        const QList<QNetworkCookie> cookies = QNetworkCookie::parseCookies(result.value(QStringLiteral("headers")).value<Headers>().header(QStringLiteral("Set-Cookie")).toLatin1());
        QVERIFY(!cookies.empty());
        for (const QNetworkCookie &cookie : cookies) {
            if (cookie.name() == m_cookieName.toLatin1()) {
                m_cookie = cookie;
                break;
            }
        }
        QVERIFY(!m_cookie.value().isEmpty());
        initTest();
    }
}

TestEngine* TestCsrfProtection::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    auto csrf = new CSRFProtection(app);
    csrf->setCookieName(m_cookieName);
    csrf->setGenericErrorMessage(QStringLiteral("denied"));
    csrf->setFormFieldName(m_fieldName);
    csrf->setHeaderName(m_headerName);
    csrf->setIgnoredNamespaces(QStringList(QStringLiteral("testns")));
    new CsrfprotectionTest(app);
    new CsrfprotectionNsTest(app);
    if (!engine->init()) {
        delete engine;
        return nullptr;
    }
    return engine;
}

void TestCsrfProtection::cleanupTestCase()
{
    delete m_engine;
}

void TestCsrfProtection::initTest()
{
    Headers headers;
    headers.setHeader(QStringLiteral("Cookie"), QString::fromLatin1(m_cookie.toRawForm(QNetworkCookie::NameAndValueOnly)));
    m_fieldValue = m_engine->createRequest(QStringLiteral("GET"), QStringLiteral("csrfprotection/test/testCsrf"), QByteArray(), headers, nullptr).value(QStringLiteral("body")).toString();
}

void TestCsrfProtection::cleanupTest()
{
    m_fieldValue.clear();
}

void TestCsrfProtection::doTest()
{
    QFETCH(QString, method);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(int, status);
    QFETCH(QByteArray, output);

    const QVariantMap result = m_engine->createRequest(method, QStringLiteral("csrfprotection/test/testCsrf"), QByteArray(), headers, &body);

    QCOMPARE(result.value(QStringLiteral("statusCode")).value<int>(), status);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}

void TestCsrfProtection::doTest_data()
{
    QTest::addColumn<QString>("method");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("output");

    for (const QString &method : {QStringLiteral("POST"), QStringLiteral("PUT"), QStringLiteral("PATCH"), QStringLiteral("DELETE")}) {
        const QString cookieValid = QString::fromLatin1(m_cookie.toRawForm(QNetworkCookie::NameAndValueOnly));
        QString cookieInvalid = cookieValid;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        auto& cookieLast = cookieInvalid[cookieInvalid.size() - 1];
#else
        QCharRef cookieLast = cookieInvalid[cookieInvalid.size() - 1];
#endif
        if (cookieLast.isDigit()) {
            if (cookieLast.unicode() < 57) {
                cookieLast.unicode()++;
            } else {
                cookieLast.unicode()--;
            }
        } else {
            if (cookieLast.isUpper()) {
                cookieLast = cookieLast.toLower();
            } else {
                cookieLast = cookieLast.toUpper();
            }
        }

        QString fieldValueInvalid = m_fieldValue;
#if (QT_VERSION >= QT_VERSION_CHECK(6, 0, 0))
        auto& fieldLast = fieldValueInvalid[fieldValueInvalid.size() - 2];
#else
        QCharRef fieldLast = fieldValueInvalid[fieldValueInvalid.size() - 2];
#endif
        if (fieldLast.isDigit()) {
            if (fieldLast.unicode() < 57) {
                fieldLast.unicode()++;
            } else {
                fieldLast.unicode()--;
            }
        } else {
            if (fieldLast.isUpper()) {
                fieldLast = fieldLast.toLower();
            } else {
                fieldLast = fieldLast.toUpper();
            }
        }

        const QString fieldValid = m_fieldName + QLatin1Char('=') + m_fieldValue;
        const QString fieldInvalid = m_fieldName + QLatin1Char('=') + fieldValueInvalid;

        Headers headers;
        headers.setContentType(QStringLiteral("application/x-www-form-urlencoded"));
        QByteArray body;

        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(absent), header(absent), field(absent)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(QStringLiteral("Cookie"), cookieValid);
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(valid), header(absent), field(absent)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(QStringLiteral("Cookie"), cookieInvalid);
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(invalid), header(absent), field(absent)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.removeHeader(QStringLiteral("Cookie"));
        headers.setHeader(m_headerName, m_fieldValue);
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(absent), header(valid), field(absent)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(QStringLiteral("Cookie"), cookieValid);
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(valid), header(valid), field(absent)").arg(method))) << method << headers << body << 200 << QByteArrayLiteral("allowed");

        headers.setHeader(QStringLiteral("Cookie"), cookieInvalid);
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(invalid), header(valid), field(absent)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(m_headerName, fieldValueInvalid);
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(invalid), header(invalid), field(absent)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(QStringLiteral("Cookie"), cookieValid);
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(valid), header(invalid), field(absent)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        body = method != u"DELETE" ? fieldValid.toLatin1() : QByteArray();
        int status = (method != u"DELETE") ? 200 : 403;
        QByteArray result = (method != u"DELETE") ? QByteArrayLiteral("allowed") : QByteArrayLiteral("denied");
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(valid), header(invalid), field(valid)").arg(method))) << method << headers << body << status << result;

        body = fieldInvalid.toLatin1();
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(valid), header(invalid), field(invalid)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(m_headerName, m_fieldValue);
        status = method == u"DELETE" ? 200 : 403;
        result = method == u"DELETE" ? QByteArrayLiteral("allowed") : QByteArrayLiteral("denied");
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(valid), header(valid), field(invalid)").arg(method))) << method << headers << body << status << result;

        headers.setHeader(QStringLiteral("Cookie"), cookieInvalid);
        body = fieldValid.toLatin1();
        QTest::newRow(qUtf8Printable(QStringLiteral("%1: cookie(invalid), header(valid), field(valid)").arg(method))) << method << headers << body << 403 << QByteArrayLiteral("denied");
    }
}

void TestCsrfProtection::detachToOnArgument()
{
    const QVariantMap result = m_engine->createRequest(QStringLiteral("POST"), QStringLiteral("csrfprotection/test/testCsrfDetachTo"), QByteArray(), Headers(), nullptr);
    QCOMPARE(result.value(QStringLiteral("statusCode")).value<int>(), 403);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), QByteArrayLiteral("detachdenied"));
}

void TestCsrfProtection::csrfIgnorArgument()
{
    const QVariantMap result = m_engine->createRequest(QStringLiteral("POST"), QStringLiteral("csrfprotection/test/testCsrfIgnore"), QByteArray(), Headers(), nullptr);
    QCOMPARE(result.value(QStringLiteral("statusCode")).value<int>(), 200);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), QByteArrayLiteral("allowed"));
}

void TestCsrfProtection::ignoreNamespace()
{
    const QVariantMap result = m_engine->createRequest(QStringLiteral("POST"), QStringLiteral("testns/testCsrf"), QByteArray(), Headers(), nullptr);
    QCOMPARE(result.value(QStringLiteral("statusCode")).value<int>(), 200);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), QByteArrayLiteral("allowed"));
}

void TestCsrfProtection::ignoreNamespaceRequired()
{
    const QVariantMap result = m_engine->createRequest(QStringLiteral("POST"), QStringLiteral("testns/testCsrfRequired"), QByteArray(), Headers(), nullptr);
    QCOMPARE(result.value(QStringLiteral("statusCode")).value<int>(), 403);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), QByteArrayLiteral("denied"));
}

void TestCsrfProtection::csrfRedirect()
{
    const QVariantMap result = m_engine->createRequest(QStringLiteral("POST"), QStringLiteral("csrfprotection/test/testCsrfRedirect"), QByteArray(), Headers(), nullptr);
    QCOMPARE(result.value(QStringLiteral("statusCode")).value<int>(), 403);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), QByteArrayLiteral("denied"));
}

QTEST_MAIN(TestCsrfProtection)

#include "testcsrfprotection.moc"
