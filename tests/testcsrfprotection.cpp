#include "coverageobject.h"

#include <Cutelyst/Application>
#include <Cutelyst/Controller>
#include <Cutelyst/Plugins/CSRFProtection/CSRFProtection>

#include <QNetworkCookie>
#include <QObject>
#include <QTest>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

class TestCsrfProtection : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestCsrfProtection(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

    void initTest() override;
    void cleanupTest() override;

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
    TestEngine *m_engine = nullptr;
    QNetworkCookie m_cookie;
    const QByteArray m_cookieName = "xsrftoken";
    const QByteArray m_fieldName  = "xsrfprotect";
    const QByteArray m_headerName = "X-MY-CSRF";
    QByteArray m_fieldValue;

    TestEngine *getEngine();

    void performTest();

    void invalidateToken(QChar &ch);
};

class CsrfprotectionTest : public Controller
{
    Q_OBJECT
public:
    explicit CsrfprotectionTest(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(testCsrf, :Local :AutoArgs)
    void testCsrf(Context *c)
    {
        c->res()->setContentType("text/plain");
        if (c->req()->isGet()) {
            c->res()->setBody(CSRFProtection::getToken(c));
        } else {
            c->res()->setBody(QByteArrayLiteral("allowed"));
        }
    }

    C_ATTR(testCsrfIgnore, :Local :AutoArgs :CSRFIgnore)
    void testCsrfIgnore(Context *c)
    {
        c->res()->setContentType("text/plain");
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }

    C_ATTR(testCsrfRedirect, :Local :AutoArgs)
    void testCsrfRedirect(Context *c) { c->res()->redirect(u"http//www.example.com"_s); }

    C_ATTR(testCsrfDetachTo, :Local :AutoArgs :CSRFDetachTo(csrfdenied))
    void testCsrfDetachTo(Context *c)
    {
        c->res()->setContentType("text/plain");
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }

    C_ATTR(csrfdenied, :Private :AutoArgs)
    void csrfdenied(Context *c)
    {
        c->res()->setContentType("text/plain");
        c->res()->setBody(QByteArrayLiteral("detachdenied"));
        c->finalize();
    }
};

class CsrfprotectionNsTest : public Controller
{
    Q_OBJECT
    C_NAMESPACE("testns")
public:
    explicit CsrfprotectionNsTest(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(testCsrf, :Local :AutoArgs)
    void testCsrf(Context *c)
    {
        c->res()->setContentType("text/plain");
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }

    C_ATTR(testCsrfRequired, :Local :AutoArgs :CSRFRequire)
    void testCsrfRequired(Context *c)
    {
        c->res()->setContentType("text/plain");
        c->res()->setBody(QByteArrayLiteral("allowed"));
    }
};

void TestCsrfProtection::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
    if (m_cookie.value().isEmpty()) {
        const auto result = m_engine->createRequest(
            "GET", u"/csrfprotection/test/testCsrf"_s, QByteArray(), Headers(), nullptr);
        const QList<QNetworkCookie> cookies =
            QNetworkCookie::parseCookies(result.headers.header("Set-Cookie"));
        QVERIFY(!cookies.empty());
        auto foundIt = std::ranges::find_if(cookies, [this](const QNetworkCookie &cookie) {
            return cookie.name() == m_cookieName;
        });
        if (foundIt != cookies.end()) {
            m_cookie = *foundIt;
        }
        QVERIFY(!m_cookie.value().isEmpty());
        initTest();
    }
    QCOMPARE(CSRFProtection::formFieldName(), m_fieldName);
}

TestEngine *TestCsrfProtection::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    auto csrf   = new CSRFProtection(app);
    csrf->setCookieName(m_cookieName);
    csrf->setGenericErrorMessage(u"denied"_s);
    csrf->setFormFieldName(m_fieldName);
    csrf->setHeaderName(m_headerName);
    csrf->setIgnoredNamespaces(QStringList(u"testns"_s));
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
    headers.setHeader("Cookie", m_cookie.toRawForm(QNetworkCookie::NameAndValueOnly));
    m_fieldValue =
        m_engine
            ->createRequest(
                "GET", u"/csrfprotection/test/testCsrf"_s, QByteArray(), headers, nullptr)
            .body;
}

void TestCsrfProtection::cleanupTest()
{
    m_fieldValue.clear();
}

void TestCsrfProtection::invalidateToken(QChar &ch)
{
    if (ch.isDigit()) {
        if (ch < u'9') {
            ch.unicode()++;
        } else {
            ch.unicode()--;
        }
    } else if (ch.isLetter()) {
        if (ch.isUpper()) {
            ch = ch.toLower();
        } else {
            ch = ch.toUpper();
        }
    } else if (ch == u'-') {
        ch = u'_';
    } else if (ch == u'_') {
        ch = u'-';
    }
}

void TestCsrfProtection::doTest()
{
    QFETCH(QByteArray, method);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(int, status);
    QFETCH(QByteArray, output);

    const auto result = m_engine->createRequest(
        method, u"/csrfprotection/test/testCsrf"_s, QByteArray(), headers, &body);

    QCOMPARE(result.statusCode, status);
    QCOMPARE(result.body, output);
}

void TestCsrfProtection::doTest_data()
{
    QTest::addColumn<QByteArray>("method");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("output");

    for (const QByteArray &method : {
             "POST",
             "PUT",
             "PATCH",
             "DELETE",
         }) {
        const auto cookieValid = m_cookie.toRawForm(QNetworkCookie::NameAndValueOnly);
        auto cookieInvalid     = QString::fromLatin1(cookieValid);
        invalidateToken(cookieInvalid[cookieInvalid.size() - 1]);

        auto fieldValueInvalid = QString::fromLatin1(m_fieldValue);
        invalidateToken(fieldValueInvalid[fieldValueInvalid.size() - 2]);

        const QByteArray fieldValid   = m_fieldName + '=' + m_fieldValue;
        const QByteArray fieldInvalid = m_fieldName + '=' + fieldValueInvalid.toLatin1();

        Headers headers;
        headers.setContentType("application/x-www-form-urlencoded");
        QByteArray body;

        QTest::newRow(qUtf8Printable(u"%1: cookie(absent), header(absent), field(absent)"_s.arg(
            QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader("Cookie", cookieValid);
        QTest::newRow(qUtf8Printable(
            u"%1: cookie(valid), header(absent), field(absent)"_s.arg(QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader("Cookie", cookieInvalid.toLatin1());
        QTest::newRow(qUtf8Printable(u"%1: cookie(invalid), header(absent), field(absent)"_s.arg(
            QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.removeHeader("Cookie");
        headers.setHeader(m_headerName, m_fieldValue);
        QTest::newRow(qUtf8Printable(
            u"%1: cookie(absent), header(valid), field(absent)"_s.arg(QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader("Cookie", cookieValid);
        QTest::newRow(qUtf8Printable(
            u"%1: cookie(valid), header(valid), field(absent)"_s.arg(QString::fromLatin1(method))))
            << method << headers << body << 200 << QByteArrayLiteral("allowed");

        headers.setHeader("Cookie", cookieInvalid.toLatin1());
        QTest::newRow(qUtf8Printable(u"%1: cookie(invalid), header(valid), field(absent)"_s.arg(
            QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(m_headerName, fieldValueInvalid.toLatin1());
        QTest::newRow(qUtf8Printable(u"%1: cookie(invalid), header(invalid), field(absent)"_s.arg(
            QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader("Cookie", cookieValid);
        QTest::newRow(qUtf8Printable(u"%1: cookie(valid), header(invalid), field(absent)"_s.arg(
            QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        body       = method != "DELETE" ? fieldValid : QByteArray();
        int status = (method != "DELETE") ? 200 : 403;
        QByteArray result =
            (method != "DELETE") ? QByteArrayLiteral("allowed") : QByteArrayLiteral("denied");
        QTest::newRow(qUtf8Printable(
            u"%1: cookie(valid), header(invalid), field(valid)"_s.arg(QString::fromLatin1(method))))
            << method << headers << body << status << result;

        body = fieldInvalid;
        QTest::newRow(qUtf8Printable(u"%1: cookie(valid), header(invalid), field(invalid)"_s.arg(
            QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");

        headers.setHeader(m_headerName, m_fieldValue);
        status = method == "DELETE" ? 200 : 403;
        result = method == "DELETE" ? QByteArrayLiteral("allowed") : QByteArrayLiteral("denied");
        QTest::newRow(qUtf8Printable(
            u"%1: cookie(valid), header(valid), field(invalid)"_s.arg(QString::fromLatin1(method))))
            << method << headers << body << status << result;

        headers.setHeader("Cookie", cookieInvalid.toLatin1());
        body = fieldValid;
        QTest::newRow(qUtf8Printable(
            u"%1: cookie(invalid), header(valid), field(valid)"_s.arg(QString::fromLatin1(method))))
            << method << headers << body << 403 << QByteArrayLiteral("denied");
    }
}

void TestCsrfProtection::detachToOnArgument()
{
    const auto result = m_engine->createRequest(
        "POST", u"/csrfprotection/test/testCsrfDetachTo"_s, QByteArray(), Headers(), nullptr);
    QCOMPARE(result.statusCode, 403);
    QCOMPARE(result.body, QByteArrayLiteral("detachdenied"));
}

void TestCsrfProtection::csrfIgnorArgument()
{
    const auto result = m_engine->createRequest(
        "POST", u"/csrfprotection/test/testCsrfIgnore"_s, QByteArray(), Headers(), nullptr);
    QCOMPARE(result.statusCode, 200);
    QCOMPARE(result.body, QByteArrayLiteral("allowed"));
}

void TestCsrfProtection::ignoreNamespace()
{
    const auto result =
        m_engine->createRequest("POST", u"/testns/testCsrf"_s, QByteArray(), Headers(), nullptr);
    QCOMPARE(result.statusCode, 200);
    QCOMPARE(result.body, QByteArrayLiteral("allowed"));
}

void TestCsrfProtection::ignoreNamespaceRequired()
{
    const auto result = m_engine->createRequest(
        "POST", u"/testns/testCsrfRequired"_s, QByteArray(), Headers(), nullptr);
    QCOMPARE(result.statusCode, 403);
    QCOMPARE(result.body, QByteArrayLiteral("denied"));
}

void TestCsrfProtection::csrfRedirect()
{
    const auto result = m_engine->createRequest(
        "POST", u"/csrfprotection/test/testCsrfRedirect"_s, QByteArray(), Headers(), nullptr);
    QCOMPARE(result.statusCode, 403);
    QCOMPARE(result.body, QByteArrayLiteral("denied"));
}

QTEST_MAIN(TestCsrfProtection)

#include "testcsrfprotection.moc"
