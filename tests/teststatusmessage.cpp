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
    void error(Context *c) { c->response()->setBody(StatusMessage::error(c, u"SM:error"_s)); }

    C_ATTR(errorTest, :Local :AutoArgs)
    void errorTest(Context *c, const QString &statusKey, const QString &errorKey)
    {
        qDebug() << "---- stash" << c->stash();
        c->response()->setBody(c->stash(errorKey).toString());
    }

    C_ATTR(status, :Local :AutoArgs)
    void status(Context *c) { c->response()->setBody(StatusMessage::status(c, u"SM:status"_s)); }

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
            c, u"SM:errorQuery"_s, ParamsMultiMap{{u"SM"_s, u"testing"_s}});
        c->response()->setBody(c->uriFor(u"/"_s, ret).toString(QUrl::FullyEncoded));
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
            c, u"SM:statusQuery"_s, ParamsMultiMap{{u"SM"_s, u"testing"_s}});
        c->response()->setBody(c->uriFor(u"/"_s, ret).toString(QUrl::FullyEncoded));
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
    TestEngine *m_engine = nullptr;
    StatusMessage *m_sm  = nullptr;

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
    headers.setHeader(QByteArrayLiteral("Cookie"), headers.header("Set-Cookie"));

    QUrl urlAux2(url + u"Test/" + m_sm->statusMsgStashKey() + QLatin1Char('/') +
                 m_sm->errorMgStashKey());

    auto body = QString::fromLatin1(result.body);
    QUrlQuery query;
    if (body.startsWith(u"http://")) {
        QUrl urlToken(body);
        QUrlQuery tokenQuery(urlToken);
        QCOMPARE(tokenQuery.queryItemValue(u"SM"_s), u"testing"_s);
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
        << u"/status/message/test/error"_s << QString{} << QString{} << QString{} << QString{}
        << QByteArrayLiteral("SM:error");
    QTest::newRow("statusmessage-error-01")
        << u"/status/message/test/error"_s << u"some_token"_s << u"sm_prefix"_s << u"sm_status"_s
        << u"sm_error"_s << QByteArrayLiteral("SM:error");

    QTest::newRow("statusmessage-status-00")
        << u"/status/message/test/status"_s << QString{} << QString{} << QString{} << QString{}
        << QByteArrayLiteral("SM:status");
    QTest::newRow("statusmessage-status-01")
        << u"/status/message/test/status"_s << u"some_token"_s << u"sm_prefix"_s << u"sm_status"_s
        << u"sm_error"_s << QByteArrayLiteral("SM:status");

    QTest::newRow("statusmessage-errorquery-00")
        << u"/status/message/test/errorQuery"_s << QString{} << QString{} << QString{} << QString{}
        << QByteArrayLiteral("SM:errorQuery");
    QTest::newRow("statusmessage-errorquery-01")
        << u"/status/message/test/errorQuery"_s << u"some_token"_s << u"sm_prefix"_s
        << u"sm_status"_s << u"sm_error"_s << QByteArrayLiteral("SM:errorQuery");

    QTest::newRow("statusmessage-statusquery-00")
        << u"/status/message/test/statusQuery"_s << QString{} << QString{} << QString{} << QString{}
        << QByteArrayLiteral("SM:statusQuery");
    QTest::newRow("statusmessage-statusquery-01")
        << u"/status/message/test/statusQuery"_s << u"some_token"_s << u"sm_prefix"_s
        << u"sm_status"_s << u"sm_error"_s << QByteArrayLiteral("SM:statusQuery");
}

QTEST_MAIN(TestStatusMessage)

#include "teststatusmessage.moc"
