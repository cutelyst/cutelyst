#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include <QTest>
#include <QObject>
#include <QUrlQuery>

#include "headers.h"
#include "coverageobject.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationuser.h>
#include <Cutelyst/Plugins/Authentication/authenticationrealm.h>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Authentication/credentialhttp.h>
#include <Cutelyst/Plugins/Authentication/minimal.h>
#include <Cutelyst/Plugins/Session/Session>

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

using namespace Cutelyst;

class TestAuthentication : public CoverageObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() {
        doTest();
    }

    void cleanupTestCase();

private:
    TestEngine *m_engine;

    TestEngine* getEngine();

    void doTest();

};

class AuthenticationTest : public Controller
{
    Q_OBJECT
public:
    AuthenticationTest(QObject *parent) : Controller(parent) {}

    C_ATTR(authenticate, :Local :AutoArgs)
    void authenticate(Context *c) {
        if (Authentication::authenticate(c)) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user, :Local :AutoArgs)
    void authenticate_user(Context *c) {
        if (Authentication::authenticate(c, c->request()->queryParameters())) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user_realm, :Local :AutoArgs)
    void authenticate_user_realm(Context *c, const QString &realm) {
        if (Authentication::authenticate(c, c->request()->queryParameters(), realm)) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }
};

void TestAuthentication::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine* TestAuthentication::getEngine()
{
    auto app = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());

    auto auth = new Authentication(app);

    auto clearStore = new StoreMinimal;
    AuthenticationUser fooUser(QStringLiteral("foo"));
    fooUser.insert(QStringLiteral("password"), QStringLiteral("123"));
    clearStore->addUser(fooUser);
    AuthenticationUser barUser(QStringLiteral("bar"));
    barUser.insert(QStringLiteral("password"), QStringLiteral("321"));
    clearStore->addUser(barUser);
    auto clearPassword = new CredentialPassword;
    clearPassword->setPasswordField(QStringLiteral("password"));
    clearPassword->setPasswordType(CredentialPassword::Clear);
    auth->addRealm(new AuthenticationRealm(clearStore, clearPassword));


    auto hashedStore = new StoreMinimal;
    fooUser.insert(QStringLiteral("password"), CredentialPassword::createPassword(QByteArrayLiteral("123"), QCryptographicHash::Sha256, 10, 10, 10));
    clearStore->addUser(fooUser);
    barUser.insert(QStringLiteral("password"), CredentialPassword::createPassword(QByteArrayLiteral("321"), QCryptographicHash::Sha256, 20, 20, 20));
    clearStore->addUser(barUser);
    auto hashedPassword = new CredentialPassword;
    hashedPassword->setPasswordField(QStringLiteral("password"));
    hashedPassword->setPasswordType(CredentialPassword::Hashed);
    auth->addRealm(new AuthenticationRealm(hashedStore, hashedPassword), QStringLiteral("hashed"));

    new Session(app);

    new AuthenticationTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}


void TestAuthentication::cleanupTestCase()
{
    delete m_engine;
}

void TestAuthentication::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    QVariantMap result = m_engine->createRequest(QStringLiteral("GET"),
                                                 urlAux.path(),
                                                 urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                 Headers(),
                                                 nullptr);

    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}

void TestAuthentication::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    // UriFor
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-test00") << QStringLiteral("/authentication/test/authenticate?") + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-test01") << QStringLiteral("/authentication/test/authenticate?") + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-test02") << QStringLiteral("/authentication/test/authenticate_user?") + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-test03") << QStringLiteral("/authentication/test/authenticate_user?") + query.toString(QUrl::FullyEncoded)
                                   << QByteArrayLiteral("fail");

}

QTEST_MAIN(TestAuthentication)

#include "testauthentication.moc"

#endif
