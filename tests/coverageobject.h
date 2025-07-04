#ifndef TEST_COVERAGE_OBJECT_H
#define TEST_COVERAGE_OBJECT_H

#include "config.h"

#include <Cutelyst/Application>
#include <Cutelyst/Context>
#include <Cutelyst/Controller>
#include <Cutelyst/Engine>
#include <Cutelyst/TestEngine>

#include <QBuffer>
#include <QObject>

using namespace Cutelyst;
using namespace Qt::Literals::StringLiterals;

class CoverageObject : public QObject
{
    Q_OBJECT
public:
    explicit CoverageObject(QObject *parent = nullptr)
        : QObject(parent)
    {
    }
    virtual void initTest() {}
    virtual void cleanupTest() {}

protected Q_SLOTS:
    void init();
    void cleanup();

private:
    void saveCoverageData();
    QString generateTestName() const;
};

class RootController : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    explicit RootController(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(rootAction, :Path :AutoArgs)
    void rootAction(Context *c) { c->response()->setBody(c->actionName()); }

    C_ATTR(rootActionOnControllerWithoutNamespace, :Local :AutoArgs)
    void rootActionOnControllerWithoutNamespace(Context *c)
    {
        c->response()->setBody(c->actionName());
    }

    C_ATTR(denied, :Local :AutoArgs)
    void denied(Context *c)
    {
        c->response()->setStatus(Response::Forbidden);
        c->response()->setBody(c->actionName());
    }

    C_ATTR(default404, :Path)
    void default404(Context *c)
    {
        c->response()->setStatus(Response::NotFound);
        c->response()->setBody("404 - Not Found."_ba);
    }

private:
    C_ATTR(Begin,)
    bool Begin(Context *) { return true; }

    C_ATTR(Auto,)
    bool Auto(Context *c)
    {
        if (!c->req()->queryParam(u"autoFalse"_s).isEmpty()) {
            c->response()->setStatus(Response::InternalServerError);
            c->response()->setBody("autoFalse"_ba);
            return false;
        }
        return true;
    }

    C_ATTR(End,)
    bool End(Context *) { return true; }
};

class TestController : public Controller
{
    Q_OBJECT
    C_NAMESPACE("///test/controller")
public:
    explicit TestController(QObject *parent)
        : Controller(parent)
    {
    }

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c)
    {
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(hello, :Local :AutoArgs)
    void hello(Context *c)
    {
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(global, :Global :AutoArgs)
    void global(Context *c)
    {
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(many, :Local :AutoArgs)
    void many(Context *c, const QStringList &args)
    {
        Q_UNUSED(args)
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(one, :Local :AutoArgs)
    void one(Context *c, const QString &one)
    {
        Q_UNUSED(one)
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(two, :Local :AutoArgs)
    void two(Context *c, const QString &one, const QString &two)
    {
        Q_UNUSED(one)
        Q_UNUSED(two)
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(manyOld, :Local :Args)
    void manyOld(Context *c)
    {
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(manyOldWithNoArgs, :Local)
    void manyOldWithNoArgs(Context *c)
    {
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(oneOld, :Local :Args(1))
    void oneOld(Context *c)
    {
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(twoOld, :Local :Args(2))
    void twoOld(Context *c)
    {
        c->response()->setBody(
            QStringLiteral("path %1 args %2")
                .arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(root, :Chained("/"))
    void root(Context *c) { c->response()->body().append(QByteArrayLiteral("/root")); }

    C_ATTR(rootItem, :Chained("root") :PathPart("item"))
    void rootItem(Context *c)
    {
        // Since root has no capture part this is never called
        c->response()->body().append(QByteArrayLiteral("/root/item"));
    }

    C_ATTR(chain, :Chained("/") :PathPart("chain") :CaptureArgs(0))
    void chain(Context *c) { c->response()->body().append(QByteArrayLiteral("/chain")); }

    C_ATTR(item, :Chained("chain"))
    void item(Context *c)
    {
        c->response()->body().append(QByteArrayLiteral("/item[MANY]/"));
        c->response()->body().append(c->request()->args().join(QLatin1Char('/')).toLatin1());
    }

    C_ATTR(itemOne, :Chained("chain") :PathPart("item") :AutoArgs)
    void itemOne(Context *c, const QString &arg)
    {
        c->response()->body().append(QByteArrayLiteral("/item[ONE]/"));
        c->response()->body().append(arg.toLatin1());
    }

    C_ATTR(midle, :Chained("chain") :AutoCaptureArgs)
    void midle(Context *c, const QString &first, const QString &second)
    {
        c->response()->body().append(QByteArrayLiteral("/midle/"));
        c->response()->body().append(first.toLatin1());
        c->response()->body().append(QByteArrayLiteral("/"));
        c->response()->body().append(second.toLatin1());
    }

    C_ATTR(midleEnd, :Chained("midle") :PathPart("end") :Args(0))
    void midleEnd(Context *c) { c->response()->body().append(QByteArrayLiteral("/end")); }

    C_ATTR(midleEndMany, :Chained("midle") :PathPart("end") :Args)
    void midleEndMany(Context *c, const QStringList &args)
    {
        c->response()->body().append(QByteArrayLiteral("/end/"));
        c->response()->body().append(args.join(QLatin1Char('/')).toLatin1());
    }

    C_ATTR(uriFor, :Global :AutoArgs)
    void uriFor(Context *c, const QStringList &args)
    {
        auto query   = c->request()->queryParameters();
        QString path = query.take(QStringLiteral("path"));
        c->response()->setBody(c->uriFor(path, args, query).toString());
    }

    C_ATTR(uriForAction, :Global :AutoArgs)
    void uriForAction(Context *c, const QStringList &args)
    {
        QStringList arguments = args;
        auto query            = c->request()->queryParameters();

        QStringList captures =
            query.take(QStringLiteral("captures")).split(QLatin1Char('/'), Qt::SkipEmptyParts);
        QString action = query.take(QStringLiteral("action"));
        QUrl uri       = c->uriForAction(action, captures, arguments, query);
        if (uri.isEmpty()) {
            c->response()->setBody(QByteArray("uriForAction not found"));
        } else {
            c->response()->setBody(uri.toString());
        }
    }
};

class TestApplication : public Application
{
    Q_OBJECT
public:
    explicit TestApplication(QObject *parent = nullptr)
        : Application(parent)
    {
        defaultHeaders() = Headers();
        // load the core translations from the build directory
        loadTranslations(u"cutelystcore"_s, QStringLiteral(CUTELYST_BUILD_DIR) + u"/Cutelyst"_s);
    }
    bool init() override
    {
        new TestController(this);

        if (m_enableRootController) {
            new RootController(this);
        }

        return true;
    }

    bool m_enableRootController = true;
};

#endif
