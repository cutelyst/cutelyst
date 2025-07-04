#ifndef DISPATCHERTEST_H
#define DISPATCHERTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/Plugins/Authentication/authentication.h>
#include <Cutelyst/Plugins/Authentication/authenticationrealm.h>
#include <Cutelyst/Plugins/Authentication/credentialpassword.h>
#include <Cutelyst/Plugins/Authentication/minimal.h>
#include <Cutelyst/Plugins/Session/session.h>
#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

#include <QDir>
#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;

class ActionRoleACL : public Controller
{
    Q_OBJECT
public:
    explicit ActionRoleACL(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(acl_admin,
           :Local
           :Does(RoleACL)
           :RequiresRole(admin)
           :ACLDetachTo(acl_denied)
           :AutoArgs)
    void acl_admin(Context *c) { c->response()->body() += QByteArrayLiteral("Ok."); }

    C_ATTR(acl_admin_editor_writer,
           :Local
           :Does(RoleACL)
           :RequiresRole(admin)
           :AllowedRole(editor)
           :AllowedRole(writer)
           :ACLDetachTo(acl_denied)
           :AutoArgs)
    void acl_admin_editor_writer(Context *c) { c->response()->body() += QByteArrayLiteral("Ok."); }

    C_ATTR(acl_editor_writer,
           :Local
           :Does(RoleACL)
           :AllowedRole(editor)
           :AllowedRole(writer)
           :ACLDetachTo(acl_denied)
           :AutoArgs)
    void acl_editor_writer(Context *c) { c->response()->body() += QByteArrayLiteral("Ok."); }

    C_ATTR(acl_denied,
           :Private)
    void acl_denied(Context *c)
    {
        c->response()->setStatus(Response::Forbidden);
        // We append the body to test if an action was visited that shouldn't
        c->response()->body() += QByteArrayLiteral("Denied.");
    }

    C_ATTR(acl_denied_detach_to_absolute,
           :Local
           :Does(RoleACL)
           :AllowedRole(editor)
           :AllowedRole(writer)
           :ACLDetachTo('/denied/role/acl/denied')
           :AutoArgs)
    void acl_denied_detach_to_absolute(Context *c)
    {
        c->response()->body() += QByteArrayLiteral("Ok.");
    }

    C_ATTR(acl_denied_detach_to_global,
           :Local
           :Does(RoleACL)
           :AllowedRole(nobody)
           :ACLDetachTo('/denied')
           :AutoArgs)
    void acl_denied_detach_to_global(Context *c)
    {
        c->response()->body() += QByteArrayLiteral("Denied.");
    }

private:
    C_ATTR(Auto,)
    bool Auto(Context *c)
    {
        bool ok = Authentication::authenticate(
            c, ParamsMultiMap{{u"id"_s, c->request()->queryParam(u"user"_s)}});
        if (!ok) {
            c->response()->body() += QByteArrayLiteral("Failed login.");
        }
        return ok;
    }
};

class DeniedRoleACL : public Controller
{
    Q_OBJECT
public:
    explicit DeniedRoleACL(QObject *parent)
        : Controller(parent)
    {
    }

private Q_SLOTS:
    void denied(Cutelyst::Context *c)
    {
        c->response()->setStatus(Response::Forbidden);
        // We append the body to test if an action was visited that shouldn't
        c->response()->body() += QByteArrayLiteral("Denied on absolute action.");
    }
};

class TestActionRoleACL : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestActionRoleACL(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() { doTest(); }

    void cleanupTestCase();

private:
    TestEngine *m_engine = nullptr;

    TestEngine *getEngine();

    void doTest();
};

void TestActionRoleACL::initTestCase()
{
    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestActionRoleACL::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("10"));

    QDir buildDir = QDir::current();
    std::ignore   = buildDir.cd(u".."_s);

    QDir current        = buildDir;
    QString pluginPaths = current.absolutePath();

    std::ignore = current.cd(u"Cutelyst/Actions/RoleACL"_s);
    pluginPaths += u';' + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Release"_s);
    pluginPaths += u';' + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Release/Cutelyst/Actions/RoleACL"_s);
    pluginPaths += u';' + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Debug"_s);
    pluginPaths += u';' + current.absolutePath();

    current     = buildDir;
    std::ignore = current.cd(u"Debug/Cutelyst/Actions/RoleACL"_s);
    pluginPaths += u';' + current.absolutePath();

    qDebug() << "setting CUTELYST_PLUGINS_DIR to" << pluginPaths;
    qputenv("CUTELYST_PLUGINS_DIR", pluginPaths.toLocal8Bit());

    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    new DeniedRoleACL(app);
    new ActionRoleACL(app);

    auto clearStore = std::make_shared<StoreMinimal>(QStringLiteral(u"id"));

    AuthenticationUser fooUser(u"foo"_s);
    fooUser.insert(u"roles"_s, QStringList{u"admin"_s});
    clearStore->addUser(fooUser);

    AuthenticationUser barUser(u"bar"_s);
    barUser.insert(u"roles"_s, QStringList{u"admin"_s, u"writer"_s});
    clearStore->addUser(barUser);

    AuthenticationUser bazUser(u"baz"_s);
    bazUser.insert(u"roles"_s, QStringList{u"editor"_s});
    clearStore->addUser(bazUser);

    auto clearPassword = std::make_shared<CredentialPassword>();
    clearPassword->setPasswordType(CredentialPassword::None);

    auto auth = new Authentication(app);
    auth->addRealm(clearStore, clearPassword);

    new Session(app);

    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestActionRoleACL::cleanupTestCase()
{
    delete m_engine;
}

void TestActionRoleACL::doTest()
{
    QFETCH(QString, url);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    auto result = m_engine->createRequest(
        "GET", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), Headers(), nullptr);

    QCOMPARE(result.body, output);
}

void TestActionRoleACL::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QByteArray>("output");

    // Path dispatcher
    QTest::newRow("roleacl-test00")
        << u"/action/role/acl/acl_admin"_s << QByteArrayLiteral("Failed login.");
    QTest::newRow("roleacl-test01")
        << u"/action/role/acl/acl_admin?user=foo"_s << QByteArrayLiteral("Ok.");
    QTest::newRow("roleacl-test02")
        << u"/action/role/acl/acl_admin?user=bar"_s << QByteArrayLiteral("Ok.");
    QTest::newRow("roleacl-test03")
        << u"/action/role/acl/acl_admin?user=baz"_s << QByteArrayLiteral("Denied.");
    QTest::newRow("roleacl-test04")
        << u"/action/role/acl/acl_admin_editor_writer?user=foo"_s << QByteArrayLiteral("Denied.");
    QTest::newRow("roleacl-test05")
        << u"/action/role/acl/acl_admin_editor_writer?user=bar"_s << QByteArrayLiteral("Ok.");
    QTest::newRow("roleacl-test06")
        << u"/action/role/acl/acl_admin_editor_writer?user=baz"_s << QByteArrayLiteral("Denied.");
    QTest::newRow("roleacl-test07")
        << u"/action/role/acl/acl_editor_writer?user=foo"_s << QByteArrayLiteral("Denied.");
    QTest::newRow("roleacl-test08")
        << u"/action/role/acl/acl_editor_writer?user=bar"_s << QByteArrayLiteral("Ok.");
    QTest::newRow("roleacl-test09")
        << u"/action/role/acl/acl_editor_writer?user=baz"_s << QByteArrayLiteral("Ok.");
    QTest::newRow("roleacl-test10") << u"/action/role/acl/acl_denied_detach_to_absolute?user=foo"_s
                                    << QByteArrayLiteral("Denied on absolute action.");
    QTest::newRow("roleacl-test11") << u"/action/role/acl/acl_denied_detach_to_global?user=bar"_s
                                    << QByteArrayLiteral("acl_denied_detach_to_global");
}

QTEST_MAIN(TestActionRoleACL)

#include "testactionroleacl.moc"

#endif
