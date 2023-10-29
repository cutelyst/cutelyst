#include "coverageobject.h"

#include <Cutelyst/Plugins/Session/Session>
#include <Cutelyst/Plugins/StatusMessage/StatusMessage>
#include <Cutelyst/View>
#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>

#include <QUrlQuery>
#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;

class StatusMessageTest : public Controller
{
    Q_OBJECT
public:
    explicit StatusMessageTest(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(error, :Local :AutoArgs)
    void error(Context *c)
    {
        c->response()->setBody(StatusMessage::error(c, QStringLiteral("SM:error")));
    }

    C_ATTR(errorTest, :Local :AutoArgs)
    void errorTest(Context *c, const QString &statusKey, const QString &errorKey)
    {
        qDebug() << "---- stash" << c->stash();
        c->response()->setBody(c->stash(errorKey).toString());
    }

    C_ATTR(status, :Local :AutoArgs)
    void status(Context *c)
    {
        c->response()->setBody(StatusMessage::status(c, QStringLiteral("SM:status")));
    }

    C_ATTR(statusTest, :Local :AutoArgs)
    void statusTest(Context *c, const QString &statusKey, const QString &errorKey)
    {
        qDebug() << "---- stash" << c->stash();
        c->response()->setBody(c->stash(statusKey).toString());
    }

    C_ATTR(errorQuery, :Local :AutoArgs)
    void errorQuery(Context *c)
    {
        ParamsMultiMap ret = StatusMessage::errorQuery(
            c,
            QStringLiteral("SM:errorQuery"),
            ParamsMultiMap{{QStringLiteral("SM"), QStringLiteral("testing")}});
        c->response()->setBody(c->uriFor(QStringLiteral("/"), ret).toString(QUrl::FullyEncoded));
    }

    C_ATTR(errorQueryTest, :Local :AutoArgs)
    void errorQueryTest(Context *c, const QString &statusKey, const QString &errorKey)
    {
        qDebug() << "---- stash" << c->stash();
        c->response()->setBody(c->stash(errorKey).toString());
    }

    C_ATTR(statusQuery, :Local :AutoArgs)
    void statusQuery(Context *c)
    {
        ParamsMultiMap ret = StatusMessage::statusQuery(
            c,
            QStringLiteral("SM:statusQuery"),
            ParamsMultiMap{{QStringLiteral("SM"), QStringLiteral("testing")}});
        c->response()->setBody(c->uriFor(QStringLiteral("/"), ret).toString(QUrl::FullyEncoded));
    }

    C_ATTR(statusQueryTest, :Local :AutoArgs)
    void statusQueryTest(Context *c, const QString &statusKey, const QString &errorKey)
    {
        qDebug() << "---- stash" << c->stash();
        c->response()->setBody(c->stash(statusKey).toString());
    }

private:
    C_ATTR(Auto,)
    bool Auto(Context *c)
    {
        StatusMessage::load(c);
        return true;
    }
};

class TestStatusMessage : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestStatusMessage(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() { doTest(); }

    void cleanupTestCase();

private:
    QString m_sessionPrefix;
    QString m_tokenParam;
    QString m_statusMsgStashKey;
    QString m_errorMsgStashKey;
    TestEngine *m_engine;
    StatusMessage *m_sm;

    TestEngine *getEngine();

    void doTest();
};

void TestStatusMessage::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestStatusMessage::getEngine()
{
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new StatusMessageTest(app);

    new Session(app);

    m_sm                = new StatusMessage(app);
    m_sessionPrefix     = m_sm->sessionPrefix();
    m_tokenParam        = m_sm->tokenParam();
    m_statusMsgStashKey = m_sm->statusMsgStashKey();
    m_errorMsgStashKey  = m_sm->errorMgStashKey();

    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestStatusMessage::cleanupTestCase()
{
    delete m_engine;
}

void TestStatusMessage::doTest()
{
    QFETCH(QString, url);
    QFETCH(QString, tokenParam);
    QFETCH(QString, sessionPrefix);
    QFETCH(QString, statusMsgStashKey);
    QFETCH(QString, errorMsgStashKey);
    QFETCH(QByteArray, output);

    if (tokenParam.isNull()) {
        m_sm->setTokenParam(m_tokenParam);
    } else {
        m_sm->setTokenParam(tokenParam);
    }
    if (sessionPrefix.isNull()) {
        m_sm->setSessionPrefix(m_sessionPrefix);
    } else {
        m_sm->setSessionPrefix(sessionPrefix);
    }
    if (statusMsgStashKey.isNull()) {
        m_sm->setStatusMsgStashKey(m_statusMsgStashKey);
    } else {
        m_sm->setStatusMsgStashKey(statusMsgStashKey);
    }
    if (errorMsgStashKey.isNull()) {
        m_sm->setErrorMgStashKey(m_errorMsgStashKey);
    } else {
        m_sm->setErrorMgStashKey(errorMsgStashKey);
    }

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        "GET", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);
    Headers headers = result.headers;
    headers.setHeader("Cookie"_qba, headers.header("Set-Cookie"));

    QUrl urlAux2(url + QLatin1String("Test/") + m_sm->statusMsgStashKey() + QLatin1Char('/') +
                 m_sm->errorMgStashKey());

    auto body = QString::fromLatin1(result.body);
    QUrlQuery query;
    if (body.startsWith(u"http://")) {
        QUrl urlToken(body);
        QUrlQuery tokenQuery(urlToken);
        QCOMPARE(tokenQuery.queryItemValue(QStringLiteral("SM")), QStringLiteral("testing"));
        query.addQueryItem(m_sm->tokenParam(), tokenQuery.queryItemValue(m_sm->tokenParam()));
    } else {
        query.addQueryItem(m_sm->tokenParam(), body);
    }
    urlAux2.setQuery(query);

    auto testResult = m_engine->createRequest(
        "GET", urlAux2.path(), urlAux2.query(QUrl::FullyEncoded).toLatin1(), headers, nullptr);

    QCOMPARE(testResult.body, output);
}

void TestStatusMessage::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("tokenParam");
    QTest::addColumn<QString>("sessionPrefix");
    QTest::addColumn<QString>("statusMsgStashKey");
    QTest::addColumn<QString>("errorMsgStashKey");
    QTest::addColumn<QByteArray>("output");

    QTest::newRow("statusmessage-error-00")
        << QStringLiteral("/status/message/test/error") << QString() << QString() << QString()
        << QString() << QByteArrayLiteral("SM:error");
    QTest::newRow("statusmessage-error-01")
        << QStringLiteral("/status/message/test/error") << QStringLiteral("some_token")
        << QStringLiteral("sm_prefix") << QStringLiteral("sm_status") << QStringLiteral("sm_error")
        << QByteArrayLiteral("SM:error");

    QTest::newRow("statusmessage-status-00")
        << QStringLiteral("/status/message/test/status") << QString() << QString() << QString()
        << QString() << QByteArrayLiteral("SM:status");
    QTest::newRow("statusmessage-status-01")
        << QStringLiteral("/status/message/test/status") << QStringLiteral("some_token")
        << QStringLiteral("sm_prefix") << QStringLiteral("sm_status") << QStringLiteral("sm_error")
        << QByteArrayLiteral("SM:status");

    QTest::newRow("statusmessage-errorquery-00")
        << QStringLiteral("/status/message/test/errorQuery") << QString() << QString() << QString()
        << QString() << QByteArrayLiteral("SM:errorQuery");
    QTest::newRow("statusmessage-errorquery-01")
        << QStringLiteral("/status/message/test/errorQuery") << QStringLiteral("some_token")
        << QStringLiteral("sm_prefix") << QStringLiteral("sm_status") << QStringLiteral("sm_error")
        << QByteArrayLiteral("SM:errorQuery");

    QTest::newRow("statusmessage-statusquery-00")
        << QStringLiteral("/status/message/test/statusQuery") << QString() << QString() << QString()
        << QString() << QByteArrayLiteral("SM:statusQuery");
    QTest::newRow("statusmessage-statusquery-01")
        << QStringLiteral("/status/message/test/statusQuery") << QStringLiteral("some_token")
        << QStringLiteral("sm_prefix") << QStringLiteral("sm_status") << QStringLiteral("sm_error")
        << QByteArrayLiteral("SM:statusQuery");
}

QTEST_MAIN(TestStatusMessage)

#include "teststatusmessage.moc"
