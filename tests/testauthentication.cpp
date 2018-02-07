#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include <QTest>
#include <QObject>
#include <QNetworkCookie>
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

    C_ATTR(authenticate_realm, :Local :AutoArgs)
    void authenticate_realm(Context *c, const QString &realm) {
        if (Authentication::authenticate(c, realm)) {
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

    C_ATTR(authenticate_user_exists, :Local :AutoArgs)
    void authenticate_user_exists(Context *c, const QString &realm) {
        if (!Authentication::userExists(c)
                && Authentication::authenticate(c, c->request()->queryParameters(), realm)
                && Authentication::userExists(c)) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user_obj, :Local :AutoArgs)
    void authenticate_user_obj(Context *c, const QString &realm) {
        if (Authentication::user(c).isNull()
                && Authentication::authenticate(c, c->request()->queryParameters(), realm)
                && !Authentication::user(c).id().isNull()) {
            c->response()->setBody(QStringLiteral("ok"));
        } else {
            c->response()->setBody(QStringLiteral("fail"));
        }
    }

    C_ATTR(authenticate_user_logout, :Local :AutoArgs)
    void authenticate_user_logout(Context *c, const QString &realm) {
        if (!Authentication::userExists(c)
                && Authentication::authenticate(c, c->request()->queryParameters(), realm)
                && Authentication::userExists(c)) {
            Authentication::logout(c);
            if (!Authentication::userExists(c)) {
                c->response()->setBody(QStringLiteral("ok"));
                return;
            }
        }
        c->response()->setBody(QStringLiteral("fail"));
    }

    C_ATTR(authenticate_user_cookie, :Local :AutoArgs)
    void authenticate_user_cookie(Context *c, const QString &realm) {
        Authentication::authenticate(c, c->request()->queryParameters(), realm);
        const auto cookie = c->response()->cookie(QByteArrayLiteral("testauthentication_exec_session")).value<QNetworkCookie>();
        if (cookie.isHttpOnly() && cookie.path() == QLatin1String("/")) {
            c->response()->setBody(QStringLiteral("ok"));
            return;
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

    auto clearStore = new StoreMinimal(QStringLiteral("id"));
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


    auto hashedStore = new StoreMinimal(QStringLiteral("id"));
    fooUser.insert(QStringLiteral("password"), CredentialPassword::createPassword(QByteArrayLiteral("123"), QCryptographicHash::Sha256, 10, 10, 10));
    hashedStore->addUser(fooUser);
    barUser.insert(QStringLiteral("password"), CredentialPassword::createPassword(QByteArrayLiteral("321"), QCryptographicHash::Sha256, 20, 20, 20));
    hashedStore->addUser(barUser);
    auto hashedPassword = new CredentialPassword;
    hashedPassword->setPasswordField(QStringLiteral("password"));
    hashedPassword->setPasswordType(CredentialPassword::Hashed);
    auth->addRealm(new AuthenticationRealm(hashedStore, hashedPassword, QStringLiteral("hashed")));

    auto nonePassword = new CredentialPassword;
    nonePassword->setPasswordField(QStringLiteral("password"));
    nonePassword->setPasswordType(CredentialPassword::None);
    auth->addRealm(new AuthenticationRealm(clearStore, nonePassword, QStringLiteral("none")));

    auto clearHttpCredential = new CredentialHttp;
    clearHttpCredential->setPasswordType(CredentialHttp::Clear);
    clearHttpCredential->setUsernameField(QStringLiteral("id"));
    auth->addRealm(new AuthenticationRealm(clearStore, clearHttpCredential, QStringLiteral("httpClear")));

    auto hashedHttpCredential = new CredentialHttp;
    hashedHttpCredential->setPasswordType(CredentialHttp::Hashed);
    hashedHttpCredential->setUsernameField(QStringLiteral("id"));
    auth->addRealm(new AuthenticationRealm(hashedStore, hashedHttpCredential, QStringLiteral("httpHashed")));

    auto noneHttpCredential = new CredentialHttp;
    noneHttpCredential->setPasswordType(CredentialHttp::None);
    noneHttpCredential->setUsernameField(QStringLiteral("id"));
    auth->addRealm(clearStore, noneHttpCredential, QStringLiteral("httpNone"));


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
    QFETCH(Headers, headers);
    QFETCH(int, status);
    QFETCH(QByteArray, output);

    QUrl urlAux(url.mid(1));

    QVariantMap result = m_engine->createRequest(QStringLiteral("GET"),
                                                 urlAux.path(),
                                                 urlAux.query(QUrl::FullyEncoded).toLatin1(),
                                                 headers,
                                                 nullptr);

    QCOMPARE(result.value(QStringLiteral("statusCode")).toInt(), status);
    QCOMPARE(result.value(QStringLiteral("body")).toByteArray(), output);
}

void TestAuthentication::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<int>("status");
    QTest::addColumn<QByteArray>("output");

    // Auth based on query data
    Headers headers;
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-test00") << QStringLiteral("/authentication/test/authenticate?") + query.toString(QUrl::FullyEncoded)
                                 << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-test01") << QStringLiteral("/authentication/test/authenticate?") + query.toString(QUrl::FullyEncoded)
                                 << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-test00") << QStringLiteral("/authentication/test/authenticate_user?") + query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-test01") << QStringLiteral("/authentication/test/authenticate_user?") + query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-test02") << QStringLiteral("/authentication/test/authenticate_user?") + query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-test03") << QStringLiteral("/authentication/test/authenticate_user?") + query.toString(QUrl::FullyEncoded)
                                      << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-realm-test00") << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") + query.toString(QUrl::FullyEncoded)
                                            << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-realm-test01") << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") + query.toString(QUrl::FullyEncoded)
                                            << headers << 200 << QByteArrayLiteral("fail");
    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("321"));
    QTest::newRow("auth-user-realm-test02") << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") + query.toString(QUrl::FullyEncoded)
                                            << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("bar"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-realm-test03") << QStringLiteral("/authentication/test/authenticate_user_realm/hashed?") + query.toString(QUrl::FullyEncoded)
                                            << headers << 200 << QByteArrayLiteral("fail");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("123"));
    QTest::newRow("auth-user-realm-test04") << QStringLiteral("/authentication/test/authenticate_user_realm/none?") + query.toString(QUrl::FullyEncoded)
                                            << headers << 200 << QByteArrayLiteral("ok");

    query.clear();
    query.addQueryItem(QStringLiteral("id"), QStringLiteral("foo"));
    query.addQueryItem(QStringLiteral("password"), QStringLiteral("3212134324234324"));
    QTest::newRow("auth-user-realm-test05") << QStringLiteral("/authentication/test/authenticate_user_realm/none?") + query.toString(QUrl::FullyEncoded)
                                            << headers << 200 << QByteArrayLiteral("ok");

    // HTTP auth
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-user-realm-test00") << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed")
                                                 << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-http-user-realm-test01") << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed")
                                                 << headers << 401 << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("321"));
    QTest::newRow("auth-http-user-realm-test02") << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed")
                                                 << headers << 200 << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("123"));
    QTest::newRow("auth-http-user-realm-test03") << QStringLiteral("/authentication/test/authenticate_user_realm/httpHashed")
                                                 << headers << 401 << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-user-realm-test04") << QStringLiteral("/authentication/test/authenticate_user_realm/httpNone")
                                                 << headers << 200 << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("3212134324234324"));
    QTest::newRow("auth-http-user-realm-test05") << QStringLiteral("/authentication/test/authenticate_user_realm/httpNone")
                                                 << headers << 200 << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-realm-test00") << QStringLiteral("/authentication/test/authenticate_realm/httpHashed")
                                            << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-http-realm-test01") << QStringLiteral("/authentication/test/authenticate_realm/httpHashed")
                                            << headers << 401 << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("321"));
    QTest::newRow("auth-http-realm-test02") << QStringLiteral("/authentication/test/authenticate_realm/httpHashed")
                                            << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("bar"), QStringLiteral("123"));
    QTest::newRow("auth-http-realm-test03") << QStringLiteral("/authentication/test/authenticate_realm/httpHashed")
                                            << headers << 401 << QByteArrayLiteral("fail");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-http-realm-test04") << QStringLiteral("/authentication/test/authenticate_realm/httpNone")
                                            << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("3212134324234324"));
    QTest::newRow("auth-http-realm-test05") << QStringLiteral("/authentication/test/authenticate_realm/httpNone")
                                            << headers << 200 << QByteArrayLiteral("ok");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-exists-test00") << QStringLiteral("/authentication/test/authenticate_user_exists/httpHashed")
                                             << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-user-test01") << QStringLiteral("/authentication/test/authenticate_user_exists/httpHashed")
                                           << headers << 401 << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-obj-test00") << QStringLiteral("/authentication/test/authenticate_user_obj/httpHashed")
                                          << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-obj-test01") << QStringLiteral("/authentication/test/authenticate_user_obj/httpHashed")
                                          << headers << 401 << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-logout-test00") << QStringLiteral("/authentication/test/authenticate_user_logout/httpHashed")
                                             << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-logout-test01") << QStringLiteral("/authentication/test/authenticate_user_logout/httpHashed")
                                             << headers << 401 << QByteArrayLiteral("fail");

    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("123"));
    QTest::newRow("auth-user-cookie-test00") << QStringLiteral("/authentication/test/authenticate_user_cookie/httpHashed")
                                             << headers << 200 << QByteArrayLiteral("ok");
    headers.clear();
    headers.setAuthorizationBasic(QStringLiteral("foo"), QStringLiteral("321"));
    QTest::newRow("auth-user-cookie-test01") << QStringLiteral("/authentication/test/authenticate_user_cookie/httpHashed")
                                             << headers << 401 << QByteArrayLiteral("fail");
}

QTEST_MAIN(TestAuthentication)

#include "testauthentication.moc"

#endif
