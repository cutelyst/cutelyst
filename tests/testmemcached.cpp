#ifndef MEMCACHEDTEST_H
#define MEMCACHEDTEST_H

#include "coverageobject.h"
#include "headers.h"

#include <Cutelyst/Plugins/Memcached/Memcached>
#include <Cutelyst/application.h>
#include <Cutelyst/controller.h>
#include <Cutelyst/headers.h>
#include <Cutelyst/upload.h>
#include <utility>

#include <QObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTest>
#include <QUrlQuery>

using namespace Cutelyst;

class TestMemcached : public CoverageObject
{
    Q_OBJECT
public:
    explicit TestMemcached(QObject *parent = nullptr)
        : CoverageObject(parent)
    {
    }

private Q_SLOTS:
    void initTestCase();

    void testController_data();
    void testController() { doTest(); }
    void testLibMemcachedVersion();

    void cleanupTestCase();

private:
    TestEngine *m_engine  = nullptr;
    QProcess *m_memcached = nullptr;
    quint16 m_memcPort    = 11211;
    QString m_memcServers;

    TestEngine *getEngine();

    void doTest();

    QUrlQuery makeKeyValQuery(const QString &key, const QString &val);
};

class MemcachedTest : public Controller
{
    Q_OBJECT
public:
    explicit MemcachedTest(QObject *parent)
        : Controller(parent)
    {
    }

    // **** Start testing setting byte array ****
    C_ATTR(setByteArray, :Local :AutoArgs)
    void setByteArray(Context *c)
    {
        auto in = getKeyVal(c);
        Memcached::MemcachedReturnType rt;
        if (Memcached::set(in.first, in.second.toUtf8(), std::chrono::minutes{1}, &rt)) {
            setValid(c);
        } else {
            setRt(c, rt);
        }
    }

    // **** Start testing getting byte array ****
    C_ATTR(getByteArray, :Local :AutoArgs)
    void getByteArray(Context *c)
    {
        auto in = getKeyVal(c);
        Memcached::MemcachedReturnType rt;
        if (Memcached::get(in.first, nullptr, &rt) == in.second.toUtf8()) {
            setValid(c);
        } else {
            setRt(c, rt);
        }
    }

    // **** Start testing set empty key ****
    C_ATTR(setEmptyKey, :Local :AutoArgs)
    void setEmptyKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::set({}, QByteArray(), std::chrono::seconds{5}, &rt));
    }

    // **** Start testing setting too long key ****
    C_ATTR(setTooLongKey, :Local :AutoArgs)
    void setTooLongKey(Context *c)
    {
        QByteArray key(500, 'a');
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::set(key, QByteArray(), std::chrono::seconds{5}, &rt));
    }

    // **** Start testing setting byte array by key ****
    C_ATTR(setByteArrayByKey, :Local :AutoArgs)
    void setByteArrayByKey(Context *c)
    {
        auto in = getKeyVal(c);
        Memcached::MemcachedReturnType rt;
        if (Memcached::setByKey(
                "myLittleGroup", in.first, in.second.toUtf8(), std::chrono::seconds{5}, &rt)) {
            setValid(c);
        } else {
            setRt(c, rt);
        }
    }

    // **** Start testing getting byte array by key ****
    C_ATTR(getByteArrayByKey, :Local :AutoArgs)
    void getByteArrayByKey(Context *c)
    {
        auto in = getKeyVal(c);
        Memcached::MemcachedReturnType rt;
        if (Memcached::getByKey("myLittleGroup", in.first, nullptr, &rt) == in.second.toUtf8()) {
            setValid(c);
        } else {
            setRt(c, rt);
        }
    }

    // **** Start testing set empty key by key ****
    C_ATTR(setEmptyKeyByKey, :Local :AutoArgs)
    void setEmptyKeyByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(
            c,
            Memcached::setByKey("myLittleGroup", {}, QByteArray(), std::chrono::seconds{5}, &rt));
    }

    // **** Start testing setting too long key by key ****
    C_ATTR(setTooLongKeyByKey, :Local :AutoArgs)
    void setTooLongKeyByKey(Context *c)
    {
        QByteArray key(500, 'a');
        Memcached::MemcachedReturnType rt;
        setValidity(
            c,
            Memcached::setByKey("myLittleGroup", key, QByteArray(), std::chrono::seconds{5}, &rt));
    }

    // **** Start testing setting QVariantList ****
    C_ATTR(setQVariantList, :Local :AutoArgs)
    void setQVariantList(Context *c)
    {
        const QVariantList list = getTestVariantList();
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::set("varList", list, std::chrono::seconds{5}, &rt));
    }

    // **** Start testing getting QVariantList ****
    C_ATTR(getQVariantList, :Local :AutoArgs)
    void getQVariantList(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList list1 = getTestVariantList();
        const QVariantList list2 = Memcached::get<QVariantList>("varList", nullptr, &rt);
        setValidity(c, list1 == list2);
    }

    // **** Start testing setting QVariantList by key ****
    C_ATTR(setQVariantListByKey, :Local :AutoArgs)
    void setQVariantListByKey(Context *c)
    {
        const QVariantList list = getTestVariantList();
        Memcached::MemcachedReturnType rt;
        setValidity(
            c, Memcached::setByKey("myListGroup", "varList2", list, std::chrono::seconds{5}, &rt));
    }

    // **** Start testing getting QVariantList ****
    C_ATTR(getQVariantListByKey, :Local :AutoArgs)
    void getQVariantListByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList list1 = getTestVariantList();
        const QVariantList list2 =
            Memcached::getByKey<QVariantList>("myListGroup", "varList2", nullptr, &rt);
        setValidity(c, list1 == list2);
    }

    // **** Start testing adding QByteArray valid ****
    C_ATTR(addByteArrayValid, :Local :AutoArgs)
    void addByteArrayValid(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(
            c,
            Memcached::add("add1", QByteArrayLiteral("Lorem ipsum"), std::chrono::seconds{5}, &rt));
    }

    // *** Start testing adding QByteArray invalid ****
    C_ATTR(addByteArrayInvalid, :Local :AutoArgs)
    void addByteArrayInvalid(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(
            c,
            Memcached::add("add1", QByteArrayLiteral("Lorem ipsum"), std::chrono::seconds{5}, &rt));
    }

    // *** Start testing get after adding QByteArray ****
    C_ATTR(getAfterAddByteArray, :Local :AutoArgs)
    void getAfterAddByteArray(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QByteArray ba1 = Memcached::get("add1", nullptr, &rt);
        const QByteArray ba2 = QByteArrayLiteral("Lorem ipsum");
        setValidity(c, ba1 == ba2);
    }

    // **** Start testing adding QVariantList valid ****
    C_ATTR(addQVariantListValid, :Local :AutoArgs)
    void addQVariantListValid(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList list = getTestVariantList();
        setValidity(c, Memcached::add("add2", list, std::chrono::seconds{2}, &rt));
    }

    // **** Start testing adding QVariantList invalid ****
    C_ATTR(addQVariantListInvalid, :Local :AutoArgs)
    void addQVariantListInvalid(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList list = getTestVariantList();
        setValidity(c, Memcached::add("add2", list, std::chrono::seconds{2}, &rt));
    }

    // **** Start testing get after add QVariantList ****
    C_ATTR(getAfterAddQVariantList, :Local :AutoArgs)
    void getAfterAddQVariantList(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList l1 = getTestVariantList();
        const QVariantList l2 = Memcached::get<QVariantList>("add2", nullptr, &rt);
        setValidity(c, l1 == l2);
    }

    // **** Start testing adding QByteArray by key valid ****
    C_ATTR(addByteArrayValidByKey, :Local :AutoArgs)
    void addByteArrayValidByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(c,
                    Memcached::addByKey("addGroup",
                                        "add3",
                                        QByteArrayLiteral("Lorem ipsum"),
                                        std::chrono::seconds{5},
                                        &rt));
    }

    // *** Start testing adding QByteArray by key invalid ****
    C_ATTR(addByteArrayInvalidByKey, :Local :AutoArgs)
    void addByteArrayInvalidByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(c,
                    Memcached::addByKey("addGruop",
                                        "add3",
                                        QByteArrayLiteral("Lorem ipsum"),
                                        std::chrono::seconds{5},
                                        &rt));
    }

    // *** Start testing get after adding QByteArray by key ****
    C_ATTR(getAfterAddByteArrayByKey, :Local :AutoArgs)
    void getAfterAddByteArrayByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QByteArray ba1 = Memcached::getByKey("addGroup", "add3", nullptr, &rt);
        const QByteArray ba2 = QByteArrayLiteral("Lorem ipsum");
        setValidity(c, ba1 == ba2);
    }

    // **** Start testing adding QVariantList by key valid ****
    C_ATTR(addQVariantListValidByKey, :Local :AutoArgs)
    void addQVariantListValidByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList list = getTestVariantList();
        setValidity(c, Memcached::addByKey("addGroup", "add4", list, std::chrono::seconds{2}, &rt));
    }

    // **** Start testing adding QVariantList by key invalid ****
    C_ATTR(addQVariantListInvalidByKey, :Local :AutoArgs)
    void addQVariantListInvalidByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList list = getTestVariantList();
        setValidity(c, Memcached::addByKey("addGroup", "add4", list, std::chrono::seconds{2}, &rt));
    }

    // **** Start testing get after add QVariantList by key ****
    C_ATTR(getAfterAddQVariantListByKey, :Local :AutoArgs)
    void getAfterAddQVariantListByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList l1 = getTestVariantList();
        const QVariantList l2 = Memcached::getByKey<QVariantList>("addGroup", "add4", nullptr, &rt);
        setValidity(c, l1 == l2);
    }

    // **** Start testing replace byte array valid ****
    C_ATTR(replaceByteArrayValid, :Local :AutoArgs)
    void replaceByteArrayValid(Context *c)
    {
        Memcached::set("replace1", QByteArrayLiteral("Lorem ipsum"), std::chrono::seconds{5});
        Memcached::MemcachedReturnType rt;
        setValidity(
            c,
            Memcached::replace(
                "replace1", QByteArrayLiteral("Lorem ipsum 2"), std::chrono::seconds{5}, &rt));
    }

    // **** Start testing replace byte array invalid ****
    C_ATTR(replaceByteArrayInvalid, :Local :AutoArgs)
    void replaceByteArrayInvalid(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(
            c,
            Memcached::replace(
                "replace2", QByteArrayLiteral("Lorem ipsum 2"), std::chrono::seconds{5}, &rt));
    }

    // **** Start testing get after replace byte array ****
    C_ATTR(getAfterReplaceByteArray, :Local :AutoArgs)
    void getAfterReplaceByteArray(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QByteArray ba1 = QByteArrayLiteral("Lorem ipsum 2");
        const QByteArray ba2 = Memcached::get("replace1", nullptr, &rt);
        setValidity(c, ba1 == ba2);
    }

    // **** Start testing replace variant list valid ****
    C_ATTR(replaceQVariantListValid, :Local :AutoArgs)
    void replaceQVariantListValid(Context *c)
    {
        QVariantList list1;
        list1.append(QVariant::fromValue<quint32>(865469));
        list1.append(QVariant::fromValue<QString>(QStringLiteral("Lorem ipsum 2")));
        list1.append(QVariant::fromValue<float>(7848.23423f));
        list1.append(QVariant::fromValue<QByteArray>(QByteArrayLiteral("Lorem ipsum 2")));
        Memcached::set("replace3", list1, 5);
        Memcached::MemcachedReturnType rt;
        const QVariantList list2 = getTestVariantList();
        setValidity(c, Memcached::replace("replace3", list2, std::chrono::seconds{5}, &rt));
    }

    // **** Start testing replace variant list invalid ****
    C_ATTR(replaceQVariantListInvalid, :Local :AutoArgs)
    void replaceQVariantListInvalid(Context *c)
    {
        const QVariantList list = getTestVariantList();
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::replace("replace4", list, std::chrono::seconds{5}, &rt));
    }

    // **** Start testing get after replace variant list ****
    C_ATTR(getAfterReplaceQVariantList, :Local :AutoArgs)
    void getAfterReplaceQVariantList(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList l1 = getTestVariantList();
        const QVariantList l2 = Memcached::get<QVariantList>("replace3", nullptr, &rt);
        setValidity(c, l1 == l2);
    }

    // **** Start testing replace byte array by key valid ****
    C_ATTR(replaceByteArrayValidByKey, :Local :AutoArgs)
    void replaceByteArrayValidByKey(Context *c)
    {
        Memcached::setByKey(
            "replaceGroup", "replace5", QByteArrayLiteral("Lorem ipsum"), std::chrono::seconds{5});
        Memcached::MemcachedReturnType rt;
        setValidity(c,
                    Memcached::replaceByKey("replaceGroup",
                                            "replace5",
                                            QByteArrayLiteral("Lorem ipsum 2"),
                                            std::chrono::seconds{5},
                                            &rt));
    }

    // **** Start testing replace byte array by key invalid ****
    C_ATTR(replaceByteArrayInvalidByKey, :Local :AutoArgs)
    void replaceByteArrayInvalidByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(c,
                    Memcached::replaceByKey("replaceGroup",
                                            "replace6",
                                            QByteArrayLiteral("Lorem ipsum 2"),
                                            std::chrono::seconds{5},
                                            &rt));
    }

    // **** Start testing get after replace byte array by key ****
    C_ATTR(getAfterReplaceByteArrayByKey, :Local :AutoArgs)
    void getAfterReplaceByteArrayByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QByteArray ba1 = QByteArrayLiteral("Lorem ipsum 2");
        const QByteArray ba2 = Memcached::getByKey("replaceGroup", "replace5", nullptr, &rt);
        setValidity(c, ba1 == ba2);
    }

    // **** Start testing replace variant list by key valid ****
    C_ATTR(replaceQVariantListValidByKey, :Local :AutoArgs)
    void replaceQVariantListValidByKey(Context *c)
    {
        QVariantList list1;
        list1.append(QVariant::fromValue<quint32>(865469));
        list1.append(QVariant::fromValue<QString>(QStringLiteral("Lorem ipsum 2")));
        list1.append(QVariant::fromValue<float>(7848.23423f));
        list1.append(QVariant::fromValue<QByteArray>(QByteArrayLiteral("Lorem ipsum 2")));
        Memcached::setByKey("replaceGroup", "replace7", list1, std::chrono::seconds{5});
        Memcached::MemcachedReturnType rt;
        const QVariantList list2 = getTestVariantList();
        setValidity(c,
                    Memcached::replaceByKey(
                        "replaceGroup", "replace7", list2, std::chrono::seconds{5}, &rt));
    }

    // **** Start testing replace variant list by key invalid ****
    C_ATTR(replaceQVariantListInvalidByKey, :Local :AutoArgs)
    void replaceQVariantListInvalidByKey(Context *c)
    {
        const QVariantList list = getTestVariantList();
        Memcached::MemcachedReturnType rt;
        setValidity(c,
                    Memcached::replaceByKey(
                        "replaceGroup", "replace8", list, std::chrono::seconds{5}, &rt));
    }

    // **** Start testing get after replace by key variant list ****
    C_ATTR(getAfterReplaceQVariantListByKey, :Local :AutoArgs)
    void getAfterReplaceQVariantListByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        const QVariantList l1 = getTestVariantList();
        const QVariantList l2 =
            Memcached::getByKey<QVariantList>("replaceGroup", "replace7", nullptr, &rt);
        setValidity(c, l1 == l2);
    }

    // **** Start testing direct remove valid ****
    C_ATTR(directRemoveValid, :Local :AutoArgs)
    void directRemoveValid(Context *c)
    {
        Memcached::set("rem1", QByteArrayLiteral("Lorem ipsum"), 300);
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::remove("rem1", &rt));
    }

    // **** Start testing direct remove invalid ****
    C_ATTR(directRemoveInvalid, :Local :AutoArgs)
    void directRemoveInvalid(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::remove("rem2", &rt));
    }

    // **** Start testing get after direct remove ****
    C_ATTR(getAfterDirectRemove, :Local :AutoArgs)
    void getAfterDirectRemove(Context *c)
    {
        const QByteArray ba1 = QByteArrayLiteral("Lorem ipsum");
        const QByteArray ba2 = Memcached::get("rem1");
        setValidity(c, ba1 == ba2);
    }

    // **** Start testing direct remove by key valid ****
    C_ATTR(directRemoveValidByKey, :Local :AutoArgs)
    void directRemoveValidByKey(Context *c)
    {
        Memcached::setByKey("remGroup", "rem3", QByteArrayLiteral("Lorem ipsum"), 300);
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::removeByKey("remGroup", "rem3", &rt));
    }

    // **** Start testing direct remove by key invalid ****
    C_ATTR(directRemoveInvalidByKey, :Local :AutoArgs)
    void directRemoveInvalidByKey(Context *c)
    {
        Memcached::MemcachedReturnType rt;
        setValidity(c, Memcached::removeByKey("remGroup", "rem4", &rt));
    }

    // **** Start testing get after direct remove by key ****
    C_ATTR(getAfterDirectRemoveByKey, :Local :AutoArgs)
    void getAfterDirectRemoveByKey(Context *c)
    {
        const QByteArray ba1 = QByteArrayLiteral("Lorem ipsum");
        const QByteArray ba2 = Memcached::getByKey("remGroup", "rem3");
        setValidity(c, ba1 == ba2);
    }

    // **** Start testing exist valid ****
    C_ATTR(existValid, :Local :AutoArgs)
    void existValid(Context *c)
    {
        Memcached::set("exist1", QByteArrayLiteral("Lorem ipsum"), 10);
        setValidity(c, Memcached::exist("exist1"));
    }

    // **** Start testing exist invalid ****
    C_ATTR(existInvalid, :Local :AutoArgs)
    void existInvalid(Context *c) { setValidity(c, Memcached::exist("exist2")); }

    // **** Start testing exist by key valid ****
    C_ATTR(existByKeyValid, :Local :AutoArgs)
    void existByKeyValid(Context *c)
    {
        Memcached::setByKey("existGroup", "exist3", QByteArrayLiteral("Lorem ipsum"), 10);
        setValidity(c, Memcached::existByKey("existGroup", "exist3"));
    }

    // **** Start testing exist by key invalid ****
    C_ATTR(existByKeyInvalid, :Local :AutoArgs)
    void existByKeyInvalid(Context *c)
    {
        setValidity(c, Memcached::existByKey("existGroup", "exist4"));
    }

    // **** Start testing icrement with initial value valid ****
    C_ATTR(incrementWithInitialValid, :Local :AutoArgs)
    void incrementWithInitialValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::incrementWithInitial("inc1", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, initial == val);
        }
    }

    // **** Start testing icrement with initial value available valid ****
    C_ATTR(incrementWithInitialAvailableValid, :Local :AutoArgs)
    void incrementWithInitialAvailableValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::incrementWithInitial("inc1", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 11 == val);
        }
    }

    // **** Start testing increment valid ****
    C_ATTR(incrementValid, :Local :AutoArgs)
    void incrementValid(Context *c)
    {
        uint64_t val = 0;
        if (!Memcached::increment("inc1", 1, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 12 == val);
        }
    }

    // **** Start test get after increment ****
    C_ATTR(getAfterIncrement, :Local :AutoArgs)
    void getAfterIncrement(Context *c)
    {
        const QByteArray ba = Memcached::get("inc1");
        uint64_t val        = ba.trimmed().toULongLong();
        setValidity(c, val == 12);
    }

    // **** Start testing increment invalid ****
    C_ATTR(incrementInvalid, :Local :AutoArgs)
    void incrementInvalid(Context *c)
    {
        uint64_t val = 0;
        setValidity(c, Memcached::increment("inc2", 1, &val));
    }

    // **** Start testing icrement with initial by key valid ****
    C_ATTR(incrementWithInitialByKeyValid, :Local :AutoArgs)
    void incrementWithInitialByKeyValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::incrementWithInitialByKey(
                "incGroup", "inc3", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, initial == val);
        }
    }

    // **** Start testing icrement with initial by key value available valid ****
    C_ATTR(incrementWithInitialByKeyAvailableValid, :Local :AutoArgs)
    void incrementWithInitialByKeyAvailableValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::incrementWithInitialByKey(
                "incGroup", "inc3", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 11 == val);
        }
    }

    // **** Start testing increment by key valid ****
    C_ATTR(incrementByKeyValid, :Local :AutoArgs)
    void incrementByKeyValid(Context *c)
    {
        uint64_t val = 0;
        if (!Memcached::incrementByKey("incGroup", "inc3", 1, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 12 == val);
        }
    }

    // **** Start test get after increment by key ****
    C_ATTR(getAfterIncrementByKey, :Local :AutoArgs)
    void getAfterIncrementByKey(Context *c)
    {
        const QByteArray ba = Memcached::getByKey("incGroup", "inc3");
        uint64_t val        = ba.trimmed().toULongLong();
        setValidity(c, val == 12);
    }

    // **** Start testing increment by key invalid ****
    C_ATTR(incrementByKeyInvalid, :Local :AutoArgs)
    void incrementByKeyInvalid(Context *c)
    {
        uint64_t val = 0;
        setValidity(c, Memcached::incrementByKey("incGroup", "inc4", 1, &val));
    }

    // **** Start testing decrement with initial value valid ****
    C_ATTR(decrementWithInitialValid, :Local :AutoArgs)
    void decrementWithInitialValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::decrementWithInitial("inc5", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, initial == val);
        }
    }

    // **** Start testing decrement with initial value available valid ****
    C_ATTR(decrementWithInitialAvailableValid, :Local :AutoArgs)
    void decrementWithInitialAvailableValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::decrementWithInitial("inc5", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 9 == val);
        }
    }

    // **** Start testing decrement valid ****
    C_ATTR(decrementValid, :Local :AutoArgs)
    void decrementValid(Context *c)
    {
        uint64_t val = 0;
        if (!Memcached::decrement("inc5", 1, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 8 == val);
        }
    }

    // **** Start test get after decrement ****
    C_ATTR(getAfterDecrement, :Local :AutoArgs)
    void getAfterDecrement(Context *c)
    {
        const QByteArray ba = Memcached::get("inc5");
        uint64_t val        = ba.trimmed().toULongLong();
        setValidity(c, val == 8);
    }

    // **** Start testing decrement invalid ****
    C_ATTR(decrementInvalid, :Local :AutoArgs)
    void decrementInvalid(Context *c)
    {
        uint64_t val = 0;
        setValidity(c, Memcached::decrement("inc6", 1, &val));
    }

    // **** Start testing decrement with initial by key value valid ****
    C_ATTR(decrementWithInitialByKeyValid, :Local :AutoArgs)
    void decrementWithInitialByKeyValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::decrementWithInitialByKey(
                "decGroup", "inc7", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, initial == val);
        }
    }

    // **** Start testing icrement with initial by key value available valid ****
    C_ATTR(decrementWithInitialByKeyAvailableValid, :Local :AutoArgs)
    void decrementWithInitialByKeyAvailableValid(Context *c)
    {
        const uint64_t initial = 10;
        uint64_t val           = 0;
        if (!Memcached::decrementWithInitialByKey(
                "decGroup", "inc7", 1, initial, std::chrono::seconds{10}, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 9 == val);
        }
    }

    // **** Start testing decrement by key valid ****
    C_ATTR(decrementByKeyValid, :Local :AutoArgs)
    void decrementByKeyValid(Context *c)
    {
        uint64_t val = 0;
        if (!Memcached::decrementByKey("decGroup", "inc7", 1, &val)) {
            setInvalid(c);
        } else {
            setValidity(c, 8 == val);
        }
    }

    // **** Start test get after decrement by key ****
    C_ATTR(getAfterDecrementByKey, :Local :AutoArgs)
    void getAfterDecrementByKey(Context *c)
    {
        const QByteArray ba = Memcached::getByKey("decGroup", "inc7");
        uint64_t val        = ba.trimmed().toULongLong();
        setValidity(c, val == 8);
    }

    // **** Start testing decrement by key invalid ****
    C_ATTR(decrementByKeyInvalid, :Local :AutoArgs)
    void decrementByKeyInvalid(Context *c)
    {
        uint64_t val = 0;
        setValidity(c, Memcached::decrementByKey("decGroup", "inc8", 1, &val));
    }

    // **** Start testing cas with a QByteArray valid ****
    C_ATTR(casValid, :Local :AutoArgs)
    void casValid(Context *c)
    {
        uint64_t cas = 0;
        Memcached::set("cas1", QByteArrayLiteral("Lorem ipsum"), std::chrono::minutes{1});
        Memcached::set("cas1", QByteArrayLiteral("Lorem ipsum 2"), std::chrono::minutes{1});
        Memcached::get("cas1", &cas);
        if (Memcached::cas(
                "cas1", QByteArrayLiteral("Lorem ipsum"), std::chrono::minutes{1}, cas)) {
            setValidity(c, Memcached::get("cas1") == QByteArrayLiteral("Lorem ipsum"));
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing cas with a QByteArray invalid ****
    C_ATTR(casInvalid, :Local :AutoArgs)
    void casInvalid(Context *c)
    {
        uint64_t cas = 0;
        Memcached::set("cas2", QByteArrayLiteral("Lorem ipsum"), std::chrono::minutes{1});
        Memcached::set("cas2", QByteArrayLiteral("Lorem ipsum 2"), std::chrono::minutes{1});
        Memcached::get("cas2", &cas);
        Memcached::set("cas2", QByteArrayLiteral("Lorem ipsum 3"), std::chrono::minutes{1});
        if (Memcached::cas(
                "cas2", QByteArrayLiteral("Lorem ipsum"), std::chrono::minutes{1}, cas)) {
            setValidity(c, Memcached::get("cas2") == QByteArrayLiteral("Lorem ipsum"));
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing cas with a QVariantList valid ****
    C_ATTR(casVariantValid, :Local :AutoArgs)
    void casVariantValid(Context *c)
    {
        const auto list1 = getTestVariantList();
        const auto list2 = getTestVariantList2();
        uint64_t cas     = 0;
        Memcached::set("cas3", list1, std::chrono::minutes{1});
        Memcached::set("cas3", list2, std::chrono::minutes{1});
        Memcached::get("cas3", &cas);
        if (Memcached::cas("cas3", list1, std::chrono::minutes{1}, cas)) {
            setValidity(c, Memcached::get<QVariantList>("cas3") == list1);
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing cas with a QVariantList invalid ****
    C_ATTR(casVariantInvalid, :Local :AutoArgs)
    void casVariantInvalid(Context *c)
    {
        const auto list1 = getTestVariantList();
        const auto list2 = getTestVariantList2();
        uint64_t cas     = 0;
        Memcached::set("cas4", list1, std::chrono::minutes{1});
        Memcached::set("cas4", list2, std::chrono::minutes{1});
        Memcached::get("cas4", &cas);
        Memcached::set("cas4", list1, std::chrono::minutes{1});
        if (Memcached::cas("cas4", list2, std::chrono::minutes{1}, cas)) {
            setValidity(c, Memcached::get<QVariantList>("cas4") == list2);
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing cas by key with a QByteArray valid ****
    C_ATTR(casByKeyValid, :Local :AutoArgs)
    void casByKeyValid(Context *c)
    {
        uint64_t cas = 0;
        Memcached::setByKey(
            "casGroup", "cas5", QByteArrayLiteral("Lorem ipsum"), std::chrono::minutes{1});
        Memcached::setByKey(
            "casGroup", "cas5", QByteArrayLiteral("Lorem ipsum 2"), std::chrono::minutes{1});
        Memcached::getByKey("casGroup", "cas5", &cas);
        if (Memcached::casByKey("casGroup",
                                "cas5",
                                QByteArrayLiteral("Lorem ipsum"),
                                std::chrono::minutes{1},
                                cas)) {
            setValidity(
                c, Memcached::getByKey("casGroup", "cas5") == QByteArrayLiteral("Lorem ipsum"));
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing cas by key with a QByteArray invalid ****
    C_ATTR(casByKeyInvalid, :Local :AutoArgs)
    void casByKeyInvalid(Context *c)
    {
        uint64_t cas = 0;
        Memcached::setByKey("casGroup", "cas6", QByteArrayLiteral("Lorem ipsum"), 60);
        Memcached::setByKey("casGroup", "cas6", QByteArrayLiteral("Lorem ipsum 2"), 60);
        Memcached::getByKey("casGroup", "cas6", &cas);
        Memcached::setByKey("casGroup", "cas6", QByteArrayLiteral("Lorem ipsum 3"), 60);
        if (Memcached::casByKey("casGroup", "cas6", QByteArrayLiteral("Lorem ipsum"), 60, cas)) {
            setValidity(
                c, Memcached::getByKey("casGroup", "cas6") == QByteArrayLiteral("Lorem ipsum"));
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing cas by key with a QVariantList valid ****
    C_ATTR(casByKeyVariantValid, :Local :AutoArgs)
    void casByKeyVariantValid(Context *c)
    {
        const auto list1 = getTestVariantList();
        const auto list2 = getTestVariantList2();
        uint64_t cas     = 0;
        Memcached::setByKey("casGroup", "cas7", list1, std::chrono::minutes{1});
        Memcached::setByKey("casGroup", "cas7", list2, std::chrono::minutes{1});
        Memcached::getByKey("casGroup", "cas7", &cas);
        if (Memcached::casByKey("casGroup", "cas7", list1, std::chrono::minutes{1}, cas)) {
            setValidity(c, Memcached::getByKey<QVariantList>("casGroup", "cas7") == list1);
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing cas by key with a QVariantList invalid ****
    C_ATTR(casByKeyVariantInvalid, :Local :AutoArgs)
    void casByKeyVariantInvalid(Context *c)
    {
        const auto list1 = getTestVariantList();
        const auto list2 = getTestVariantList2();
        uint64_t cas     = 0;
        Memcached::setByKey("casGroup", "cas8", list1, std::chrono::minutes{1});
        Memcached::setByKey("casGroup", "cas8", list2, std::chrono::minutes{1});
        Memcached::getByKey("casGroup", "cas8", &cas);
        Memcached::setByKey("casGroup", "cas8", list1, std::chrono::minutes{1});
        if (Memcached::casByKey("casGroup", "cas8", list2, std::chrono::minutes{1}, cas)) {
            setValidity(c, Memcached::getByKey<QVariantList>("casGroup", "cas8") == list2);
        } else {
            setInvalid(c);
        }
    }

    // **** Start testing mget valid ****
    C_ATTR(mgetValid, :Local :AutoArgs)
    void mgetValid(Context *c)
    {
        const auto h1 = getTestHash("vala");
        auto i        = h1.constBegin();
        while (i != h1.constEnd()) {
            Memcached::set(i.key(), i.value(), std::chrono::minutes{1});
            ++i;
        }
        const auto h2 = Memcached::mget(h1.keys());
        setValidity(c, h1 == h2);
    }

    // **** Start testing mget by key valid ****
    C_ATTR(mgetByKeyValid, :Local :AutoArgs)
    void mgetByKeyValid(Context *c)
    {
        const auto h1 = getTestHash("valb");
        auto i        = h1.constBegin();
        while (i != h1.constEnd()) {
            Memcached::setByKey("mgetGroup", i.key(), i.value(), std::chrono::minutes{1});
            ++i;
        }
        const auto h2 = Memcached::mgetByKey("mgetGroup", h1.keys());
        setValidity(c, h1 == h2);
    }

    // **** Start testing mget variant valid
    C_ATTR(mgetVariantValid, :Local :AutoArgs)
    void mgetVariantValid(Context *c)
    {
        const auto h1 = getTestHashList("valc");
        auto i        = h1.constBegin();
        while (i != h1.constEnd()) {
            Memcached::set(i.key(), i.value(), std::chrono::minutes{1});
            ++i;
        }
        const auto h2 = Memcached::mget<QVariantList>(h1.keys());
        setValidity(c, h1 == h2);
    }

    // **** Start testing mget by key variant valid
    C_ATTR(mgetByKeyVariantValid, :Local :AutoArgs)
    void mgetByKeyVariantValid(Context *c)
    {
        const auto h1 = getTestHashList("vald");
        auto i        = h1.constBegin();
        while (i != h1.constEnd()) {
            Memcached::setByKey("mgetGroup", i.key(), i.value(), std::chrono::minutes{1});
            ++i;
        }
        const auto h2 = Memcached::mgetByKey<QVariantList>("mgetGroup", h1.keys());
        setValidity(c, h1 == h2);
    }

    // **** Start testing flush
    C_ATTR(flush, :Local :AutoArgs)
    void flush(Context *c)
    {
        const QByteArray key = "flushKey";
        Memcached::set(key, QByteArrayLiteral("Lorem ipsum"), std::chrono::minutes{5});
        if (Memcached::flush(0)) {
            setValidity(c, Memcached::get(key).isNull());
        } else {
            setInvalid(c);
        }
    }

private:
    std::pair<QByteArray, QString> getKeyVal(Context *c)
    {
        std::pair<QByteArray, QString> pair;

        pair.first  = c->req()->queryParam(QStringLiteral("key")).toLatin1();
        pair.second = c->req()->queryParam(QStringLiteral("val"));

        return pair;
    }

    void setValidity(Context *c, bool ok)
    {
        if (ok) {
            setValid(c);
        } else {
            setInvalid(c);
        }
    }

    void setValid(Context *c) { c->res()->setBody(QByteArrayLiteral("valid")); }

    void setInvalid(Context *c) { c->res()->setBody(QByteArrayLiteral("invalid")); }

    void setRt(Context *c, Memcached::MemcachedReturnType rt)
    {
        c->res()->setBody(Memcached::errorString(c, rt));
    }

    QVariantList getTestVariantList()
    {
        QVariantList list;
        list.append(QVariant::fromValue<quint32>(2345234));
        list.append(QVariant::fromValue<QString>(QStringLiteral("Lorem ipsum")));
        list.append(QVariant::fromValue<float>(23423.23423f));
        list.append(QVariant::fromValue<QByteArray>(QByteArrayLiteral("Lorem ipsum")));
        QVariantMap map{{QStringLiteral("val1"), QVariant::fromValue<qint64>(2983479237492)},
                        {QStringLiteral("val2"),
                         QVariant::fromValue<QString>(QStringLiteral("Dolor sit amet"))},
                        {QStringLiteral("val3"), QVariant::fromValue<float>(9832.002f)}};
        list.append(QVariant::fromValue<QVariantMap>(map));
        return list;
    }

    QVariantList getTestVariantList2()
    {
        QVariantList list;
        list.append(QVariant::fromValue<quint64>(1232345234));
        list.append(QVariant::fromValue<QString>(QStringLiteral("Lorem ipsum 2")));
        list.append(QVariant::fromValue<float>(2342.23423f));
        list.append(QVariant::fromValue<QByteArray>(QByteArrayLiteral("Lorem ipsum 3")));
        return list;
    }

    QHash<QByteArray, QByteArray> getTestHash(const QByteArray &prefix = "val")
    {
        QHash<QByteArray, QByteArray> hash;
        hash.insert(prefix + '1', QByteArrayLiteral("Lorem ipsum"));
        hash.insert(prefix + '2', QByteArrayLiteral("dolor sit amet"));
        hash.insert(prefix + '3', QByteArrayLiteral("consectetur adipiscing elit"));
        hash.insert(prefix + '4', QByteArrayLiteral("sed do eiusmod tempor"));
        hash.insert(prefix + '5', QByteArrayLiteral("incididunt ut labore et dolore"));
        hash.insert(prefix + '6', QByteArrayLiteral("magna aliqua"));
        return hash;
    }

    QHash<QByteArray, QVariantList> getTestHashList(const QByteArray &prefix = "val")
    {
        QHash<QByteArray, QVariantList> hash;
        for (int i = 0; i < 6; ++i) {
            hash.insert(prefix + QByteArray::number(i), getTestVariantList2());
        }
        return hash;
    }
};

void TestMemcached::initTestCase()
{
    m_memcServers = QString::fromLocal8Bit(qgetenv("CUTELYST_MEMCACHED_TEST_SERVERS"));

    // if no memcached server is defined by environment variable,
    // we will start our own server
    if (m_memcServers.isEmpty()) {
        // QTcpServer can automatically select a free port, so we abuse it to find a free one
        // that we will use for memcached
        auto portTestServer = new QTcpServer(this);
        QVERIFY(portTestServer->listen());
        m_memcPort = portTestServer->serverPort();
        delete portTestServer;

        QString memcBinFilePath = QStandardPaths::findExecutable(QStringLiteral("memcached"));
        if (memcBinFilePath.isEmpty()) {
            memcBinFilePath = QStandardPaths::findExecutable(
                QStringLiteral("memcached"),
                {QStringLiteral("/usr/sbin"), QStringLiteral("/sbin")});
        }

        QVERIFY2(!memcBinFilePath.isEmpty(),
                 "Can not find memcached executable. Please check if memcached is available. If "
                 "memcached is not available in the default paths, please use the environment "
                 "variable CUTELYST_MEMCACHED_TEST_SERVERS to set a memcached server that you have "
                 "started by your own before running this tests.");

        QStringList memcArgs;
        memcArgs << QStringLiteral("-p") << QString::number(m_memcPort) << QStringLiteral("-U")
                 << QString::number(m_memcPort);

        m_memcached = new QProcess(this);
        m_memcached->setProgram(memcBinFilePath);
        m_memcached->setArguments(memcArgs);

        qDebug(
            "Starting %s %s", qUtf8Printable(memcBinFilePath), qUtf8Printable(memcArgs.join(u" ")));
        m_memcached->start();

        // wait one second for the memcached server to start
        QTest::qSleep(1000);

        // check if memcached is up and running
        QTcpSocket testSocket;
        testSocket.connectToHost(QStringLiteral("127.0.0.1"), m_memcPort);
        QVERIFY(testSocket.waitForConnected(5000));
        QCOMPARE(testSocket.state(), QAbstractSocket::ConnectedState);

        m_memcServers = QStringLiteral("localhost,") + QString::number(m_memcPort);
    }

    m_engine = getEngine();
    QVERIFY(m_engine);
}

TestEngine *TestMemcached::getEngine()
{
    qputenv("RECURSION", QByteArrayLiteral("100"));
    auto app    = new TestApplication;
    auto engine = new TestEngine(app, QVariantMap());
    auto plugin = new Memcached(app);
    QVariantMap pluginConfig{{QStringLiteral("binary_protocol"), true},
                             {QStringLiteral("compression"), true},
                             {QStringLiteral("compression_threshold"), 10},
                             {QStringLiteral("servers"), m_memcServers}};
    plugin->setDefaultConfig(pluginConfig);
    new MemcachedTest(app);
    if (!engine->init()) {
        return nullptr;
    }
    return engine;
}

void TestMemcached::cleanupTestCase()
{
    delete m_engine;
    if (m_memcached) {
        m_memcached->terminate();
        m_memcached->waitForFinished();
    }
    delete m_memcached;
}

void TestMemcached::doTest()
{
    QFETCH(QString, url);
    QFETCH(Headers, headers);
    QFETCH(QByteArray, body);
    QFETCH(QByteArray, output);

    QUrl urlAux(url);

    const auto result = m_engine->createRequest(
        "POST", urlAux.path(), urlAux.query(QUrl::FullyEncoded).toLatin1(), headers, &body);

    QCOMPARE(result.body, output);
}

QUrlQuery TestMemcached::makeKeyValQuery(const QString &key, const QString &val)
{
    QUrlQuery q;
    q.addQueryItem(QStringLiteral("key"), key);
    q.addQueryItem(QStringLiteral("val"), val);
    return q;
}

void TestMemcached::testController_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<Headers>("headers");
    QTest::addColumn<QByteArray>("body");
    QTest::addColumn<QByteArray>("output");

    Headers headers;
    headers.setContentType("application/x-www-form-urlencoded");
    QUrlQuery q;

    const QMap<QString, QString> simpleInput{
        {QStringLiteral("asdfjk"), QStringLiteral("lkkljad")},
        {QStringLiteral("knda"), QStringLiteral("kjkjadsf")},
        {QStringLiteral("kljn"), QStringLiteral("kjda lkjadsf")},
        {QStringLiteral("lka54"), QStringLiteral("kjqkjd")},
        {QStringLiteral("adsfo"), QStringLiteral("dafljk")},
        {QStringLiteral("yxcfasdef"), QStringLiteral("lknkajsdnbfjakdsf")},
        {QStringLiteral("asdfhbdfjhavbsdfkagfdsfvbdjsafasfgewuoig23487bfjlahb3q2r4elfbq2w35gbawfbq2"
                        "35luqz2q3lu4vwefjlhwv32423bjhblje"),
         QStringLiteral("nbjkbadsfauwiz34gt723qbc68rw7o wvegarlzt78w468o23 328 bv287468bv "
                        "lawerbw37a4v6283qopvb3 log3q2o4b723874vboualwe")},
        {QStringLiteral("asdfasdf"), QStringLiteral("lkjasdalfasdhfasdf")}};

    // **** Start testing setting byte array ****
    // tests static bool Memcached::set(const QString &key, const QByteArray &value, time_t
    // expiration, MemcachedReturnType *returnType = nullptr)
    int count = 0;
    auto sii  = simpleInput.constBegin();
    while (sii != simpleInput.constEnd()) {
        q.clear();
        q = makeKeyValQuery(sii.key(), sii.value());
        QTest::newRow(QStringLiteral("setByteArray-%1").arg(count).toUtf8().constData())
            << QStringLiteral("/memcached/test/setByteArray?") + q.toString(QUrl::FullyEncoded)
            << headers << QByteArray() << QByteArrayLiteral("valid");
        ++sii;
        count++;
    }

    // **** Start testing getting byte array ****
    // tests static QByteArray Memcached::get(const QString &key, quint64 *cas = nullptr,
    // MemcachedReturnType *returnType = nullptr)
    count = 0;
    sii   = simpleInput.constBegin();
    while (sii != simpleInput.constEnd()) {
        q = makeKeyValQuery(sii.key(), sii.value());
        QTest::newRow(QStringLiteral("getByteArray-%1").arg(count).toUtf8().constData())
            << QStringLiteral("/memcached/test/getByteArray?") + q.toString(QUrl::FullyDecoded)
            << headers << QByteArray() << QByteArrayLiteral("valid");
        ++sii;
        count++;
    }

    // **** Start testing set empty key ****
    // tests static bool Memcached::set(const QString &key, const QByteArray &value, time_t
    // expiration, MemcachedReturnType *returnType = nullptr)
    QTest::newRow("setEmptyKey") << QStringLiteral("/memcached/test/setEmptyKey") << headers
                                 << QByteArray() << QByteArrayLiteral("invalid");

    // **** Start testing set too long key ****
    // tests static bool Memcached::set(const QString &key, const QByteArray &value, time_t
    // expiration, MemcachedReturnType *returnType = nullptr)
    QTest::newRow("setTooLongKey") << QStringLiteral("/memcached/test/setTooLongKey") << headers
                                   << QByteArray() << QByteArrayLiteral("invalid");

    // **** Start testing setting byte array by key ****
    // tests static bool Memcached::setByKey(const QString &groupKey, const QString &key, const
    // QByteArray &value, time_t expiration, MemcachedReturnType *returnType = nullptr)
    count = 0;
    sii   = simpleInput.constBegin();
    while (sii != simpleInput.constEnd()) {
        q.clear();
        q = makeKeyValQuery(sii.key(), sii.value());
        QTest::newRow(QStringLiteral("setByteArrayByKey-%1").arg(count).toUtf8().constData())
            << QStringLiteral("/memcached/test/setByteArrayByKey?") + q.toString(QUrl::FullyEncoded)
            << headers << QByteArray() << QByteArrayLiteral("valid");
        ++sii;
        count++;
    }

    // **** Start testing getting byte array by key ****
    // tests static QByteArray Memcached::getByKey(const QString &groupKey, const QString &key,
    // quint64 *cas = nullptr, MemcachedReturnType *returnType = nullptr);
    count = 0;
    sii   = simpleInput.constBegin();
    while (sii != simpleInput.constEnd()) {
        q = makeKeyValQuery(sii.key(), sii.value());
        QTest::newRow(QStringLiteral("getByteArrayByKey-%1").arg(count).toUtf8().constData())
            << QStringLiteral("/memcached/test/getByteArrayByKey?") + q.toString(QUrl::FullyDecoded)
            << headers << QByteArray() << QByteArrayLiteral("valid");
        ++sii;
        count++;
    }

    const QVector<std::pair<QString, QByteArray>> testVect{
        {QStringLiteral("setEmptyKeyByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("setTooLongKeyByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("setQVariantList"), QByteArrayLiteral("valid")},
        {QStringLiteral("getQVariantList"), QByteArrayLiteral("valid")},
        {QStringLiteral("setQVariantListByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("getQVariantListByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("addByteArrayValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("addByteArrayInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterAddByteArray"), QByteArrayLiteral("valid")},
        {QStringLiteral("addQVariantListValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("addQVariantListInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterAddQVariantList"), QByteArrayLiteral("valid")},
        {QStringLiteral("addByteArrayValidByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("addByteArrayInvalidByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterAddByteArrayByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("addQVariantListValidByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("addQVariantListInvalidByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterAddQVariantListByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceByteArrayValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceByteArrayInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterReplaceByteArray"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceQVariantListValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceQVariantListInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterReplaceQVariantList"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceByteArrayValidByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceByteArrayInvalidByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterReplaceByteArrayByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceQVariantListValidByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("replaceQVariantListInvalidByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterReplaceQVariantListByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("directRemoveValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("directRemoveInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterDirectRemove"), QByteArrayLiteral("invalid")},
        {QStringLiteral("directRemoveValidByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("directRemoveInvalidByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("getAfterDirectRemoveByKey"), QByteArrayLiteral("invalid")},
        {QStringLiteral("existValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("existInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("existByKeyValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("existByKeyInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("incrementWithInitialValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("incrementWithInitialAvailableValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("incrementValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("getAfterIncrement"), QByteArrayLiteral("valid")},
        {QStringLiteral("incrementInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("incrementWithInitialByKeyValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("incrementWithInitialByKeyAvailableValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("incrementByKeyValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("getAfterIncrementByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("incrementByKeyInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("decrementWithInitialValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("decrementWithInitialAvailableValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("decrementValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("getAfterDecrement"), QByteArrayLiteral("valid")},
        {QStringLiteral("decrementInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("decrementWithInitialByKeyValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("decrementWithInitialByKeyAvailableValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("decrementByKeyValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("getAfterDecrementByKey"), QByteArrayLiteral("valid")},
        {QStringLiteral("decrementByKeyInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("casValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("casInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("casVariantValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("casVariantInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("casByKeyValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("casByKeyInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("casByKeyVariantValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("casByKeyVariantInvalid"), QByteArrayLiteral("invalid")},
        {QStringLiteral("mgetValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("mgetByKeyValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("mgetVariantValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("mgetByKeyVariantValid"), QByteArrayLiteral("valid")},
        {QStringLiteral("flush"), QByteArrayLiteral("valid")}};

    for (const std::pair<QString, QByteArray> &test : testVect) {
        QTest::newRow(test.first.toLocal8Bit().constData())
            << QStringLiteral("/memcached/test/") + test.first << headers << QByteArray()
            << test.second;
    }
}

void TestMemcached::testLibMemcachedVersion()
{
    const QVersionNumber version = Memcached::libMemcachedVersion();
    qDebug() << "libMemcached version:" << version.toString();
    QVERIFY(!version.isNull());
}

QTEST_MAIN(TestMemcached)

#include "testmemcached.moc"

#endif // MEMCACHEDTEST_H
