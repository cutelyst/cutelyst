#ifndef _TEST_COVERAGE_OBJECT_H
#define _TEST_COVERAGE_OBJECT_H

#include <QObject>
#include <Cutelyst/Engine>
#include <Cutelyst/Application>
#include <Cutelyst/Controller>
#include <Cutelyst/Context>

using namespace Cutelyst;

class CoverageObject : public QObject
{
    Q_OBJECT
public:
    CoverageObject(QObject *parent = 0) : QObject(parent) {}
    virtual void initTest() {}
    virtual void cleanupTest() {}
protected Q_SLOTS:
    void init() ;
    void cleanup();
private:
    void saveCoverageData();
    QString generateTestName() const;
};

class TestEngine : public Engine
{
    Q_OBJECT
public:
    explicit TestEngine(const QVariantMap &opts, QObject *parent = 0);

    virtual int workerId() const override;

    virtual int workerCore() const override;

    QVariantMap createRequest(const QString &method, const QString &path, const QByteArray &query, const Headers &headers, QByteArray *body);

    virtual bool finalizeHeaders(Context *ctx) override;
protected:
    virtual qint64 doWrite(Context *c, const char *data, qint64 len, void *engineData);
    virtual bool init();

private:
    QByteArray m_responseData;
    QByteArray m_status;
    Headers m_headers;
};

class RootController : public Controller
{
    Q_OBJECT
    C_NAMESPACE("")
public:
    RootController(QObject *parent) : Controller(parent) {}

    C_ATTR(rootActionOnControllerWithoutNamespace, :Local :AutoArgs)
    void rootActionOnControllerWithoutNamespace(Context *c) {
        c->response()->setBody(c->actionName());
    }

private:
    C_ATTR(Begin,)
    bool Begin(Context *) { return true; }

    C_ATTR(Auto,)
    bool Auto(Context *) { return true; }

    C_ATTR(End,)
    bool End(Context *) { return true; }
};

class TestController : public Controller
{
    Q_OBJECT
public:
    TestController(QObject *parent) : Controller(parent) {}

    C_ATTR(index, :Path :AutoArgs)
    void index(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(hello, :Local :AutoArgs)
    void hello(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(global, :Global :AutoArgs)
    void global(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(many, :Local :AutoArgs)
    void many(Context *c, const QStringList &args) {
        Q_UNUSED(args)
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(one, :Local :AutoArgs)
    void one(Context *c, const QString &one) {
        Q_UNUSED(one)
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(two, :Local :AutoArgs)
    void two(Context *c, const QString &one, const QString &two) {
        Q_UNUSED(one)
        Q_UNUSED(two)
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(manyOld, :Local :Args)
    void manyOld(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(manyOldWithNoArgs, :Local)
    void manyOldWithNoArgs(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(oneOld, :Local :Args(1))
    void oneOld(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }

    C_ATTR(twoOld, :Local :Args(2))
    void twoOld(Context *c) {
        c->response()->setBody(QStringLiteral("path /%1 args %2").arg(c->request()->path(), c->request()->args().join(QLatin1Char('/'))));
    }


    C_ATTR(root, :Chained("/"))
    void root(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/root"));
    }

    C_ATTR(rootItem, :Chained("root") :PathPart("item"))
    void rootItem(Context *c) {
        // Since root has no capture part this is never called
        c->response()->body().append(QByteArrayLiteral("/root/item"));
    }


    C_ATTR(chain, :Chained("/") :PathPart("chain") :CaptureArgs(0))
    void chain(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/chain"));
    }

    C_ATTR(item, :Chained("chain"))
    void item(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/item[MANY]/"));
        c->response()->body().append(c->request()->args().join(QLatin1Char('/')).toLatin1());
    }

    C_ATTR(itemOne, :Chained("chain") :PathPart("item") :AutoArgs)
    void itemOne(Context *c, const QString &arg) {
        c->response()->body().append(QByteArrayLiteral("/item[ONE]/"));
        c->response()->body().append(arg.toLatin1());
    }

    C_ATTR(midle, :Chained("chain") :AutoCaptureArgs)
    void midle(Context *c, const QString &first, const QString &second) {
        c->response()->body().append(QByteArrayLiteral("/midle/"));
        c->response()->body().append(first.toLatin1());
        c->response()->body().append(QByteArrayLiteral("/"));
        c->response()->body().append(second.toLatin1());
    }

    C_ATTR(midleEnd, :Chained("midle") :PathPart("end") :Args(0))
    void midleEnd(Context *c) {
        c->response()->body().append(QByteArrayLiteral("/end"));
    }

    C_ATTR(midleEndMany, :Chained("midle") :PathPart("end") :Args)
    void midleEndMany(Context *c, const QStringList &args) {
        c->response()->body().append(QByteArrayLiteral("/end/"));
        c->response()->body().append(args.join(QLatin1Char('/')).toLatin1());
    }

    C_ATTR(uriFor, :Global :AutoArgs)
    void uriFor(Context *c, const QStringList &args) {
        auto query = c->request()->queryParameters();
        QString path = query.take(QStringLiteral("path"));
        c->response()->setBody(c->uriFor(path, args, query).toString());
    }

    C_ATTR(uriForAction, :Global :AutoArgs)
    void uriForAction(Context *c, const QStringList &args) {
        QStringList arguments = args;
        auto query = c->request()->queryParameters();
        QStringList captures = query.take(QStringLiteral("captures")).split(QLatin1Char('/'), QString::SkipEmptyParts);
        QString action = query.take(QStringLiteral("action"));
        QUrl uri = c->uriForAction(action, captures, arguments, query);
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
    TestApplication(QObject *parent = 0) : Application(parent)
    {
        defaultHeaders() = Headers();
    }
    virtual bool init() {
        new TestController(this);
        new RootController(this);

        return true;
    }
};

#endif
