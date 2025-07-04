#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>

#include <QObject>
#include <QTest>

using namespace Cutelyst;

class ControllerTest : public CoverageObject
{
    Q_OBJECT
public:
    explicit ControllerTest(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

    void initTest() override;
    void cleanupTest() override;

private Q_SLOTS:
    void testController_data();
    void testController();

private:
    TestApplication *m_app = nullptr;
    TestEngine *m_engine   = nullptr;
};

class ApiV1Users : public Controller
{
    Q_OBJECT
};

class Use_Some_Underscores : public Controller
{
    Q_OBJECT
};

class UppercaseREST : public Controller
{
    Q_OBJECT
};

namespace ApiV1 {
class NamespacedController : public Controller
{
    Q_OBJECT
};
} // namespace ApiV1

void ControllerTest::initTest()
{
    m_app    = new TestApplication;
    m_engine = new TestEngine(m_app, QVariantMap());
}

void ControllerTest::cleanupTest()
{
    delete m_app;
}

void ControllerTest::testController_data()
{
    QTest::addColumn<Controller *>("controller");
    QTest::addColumn<QString>("expectedNamespace");

    QTest::newRow("CamelCase") << static_cast<Controller *>(new ApiV1Users()) << "api/v1/users";

    QTest::newRow("Underscores") << static_cast<Controller *>(new Use_Some_Underscores())
                                 << "use_some_underscores";

    QTest::newRow("Consecutive uppercase")
        << static_cast<Controller *>(new UppercaseREST()) << "uppercase/rest";

    QTest::newRow("Namespaced") << static_cast<Controller *>(new ApiV1::NamespacedController())
                                << "api/v1/namespaced/controller";
}

void ControllerTest::testController()
{
    QFETCH(Controller *, controller);
    QFETCH(QString, expectedNamespace);

    controller->setParent(m_app);
    QVERIFY(m_engine->init());

    QCOMPARE(controller->ns(), expectedNamespace);
}

QTEST_MAIN(ControllerTest)

#include "testcontroller.moc"
