#ifndef PAGINATIONTEST_H
#define PAGINATIONTEST_H

#include "Cutelyst/Plugins/Utils/Pagination/Pagination"
#include "coverageobject.h"

#include <QtCore/QObject>
#include <QtTest/QTest>

using namespace Cutelyst;

class TestPagination : public CoverageObject
{
    Q_OBJECT
private Q_SLOTS:
    void testPagination();
    void testPagination2();
    void testPagination3();
    void testPaginationZeroPageLinks();
    void testPaginationDisabledLastPageLink();
};

void TestPagination::testPagination()
{
    int numberOfItems = 100;
    int itemsPerPage  = 5;
    int currentPage   = 3;
    int pageLinks     = 3;
    Pagination pagination(numberOfItems, itemsPerPage, currentPage, pageLinks);

    QCOMPARE(pagination.limit(), 5);
    QCOMPARE(pagination.offset(), 10);
    QCOMPARE(pagination.currentPage(), 3);
    QCOMPARE(pagination.lastPage(), 20);
    QCOMPARE(pagination.numberOfItems(), numberOfItems);
    QCOMPARE(pagination.enableFirst(), true);
    QCOMPARE(pagination.enableLast(), true);

    QVector<int> pages = {1, 2, 3, 4, 5, 6, 7};
    QCOMPARE(pagination.pages(), pages);
}

void TestPagination::testPagination3()
{
    int numberOfItems = 100;
    int itemsPerPage  = 5;
    int currentPage   = 1;
    int pageLinks     = 3;
    Pagination pagination(numberOfItems, itemsPerPage, currentPage, pageLinks);

    QCOMPARE(pagination.limit(), 5);
    QCOMPARE(pagination.offset(), 0);
    QCOMPARE(pagination.currentPage(), currentPage);
    QCOMPARE(pagination.lastPage(), 20);
    QCOMPARE(pagination.numberOfItems(), numberOfItems);
    QCOMPARE(pagination.enableFirst(), false);
    QCOMPARE(pagination.enableLast(), true);

    QVector<int> pages = {1, 2, 3, 4, 5, 6, 7};
    QCOMPARE(pagination.pages(), pages);
}

void TestPagination::testPagination2()
{
    int numberOfItems = 100;
    int itemsPerPage  = 5;
    int currentPage   = 0;
    int pageLinks     = 3;
    Pagination pagination(numberOfItems, itemsPerPage, currentPage, pageLinks);

    QCOMPARE(pagination.limit(), 5);
    QCOMPARE(pagination.offset(), 0);
    QCOMPARE(pagination.currentPage(), 1);
    QCOMPARE(pagination.lastPage(), 20);
    QCOMPARE(pagination.numberOfItems(), numberOfItems);
    QCOMPARE(pagination.enableFirst(), false);
    QCOMPARE(pagination.enableLast(), true);

    QVector<int> pages = {1, 2, 3, 4, 5, 6, 7};
    QCOMPARE(pagination.pages(), pages);
}

void TestPagination::testPaginationZeroPageLinks()
{
    int numberOfItems = 100;
    int itemsPerPage  = 5;
    int currentPage   = 0;
    int pageLinks     = 0;
    Pagination pagination(numberOfItems, itemsPerPage, currentPage, pageLinks);

    QCOMPARE(pagination.limit(), 5);
    QCOMPARE(pagination.offset(), 0);
    QCOMPARE(pagination.currentPage(), 1);
    QCOMPARE(pagination.lastPage(), 20);
    QCOMPARE(pagination.numberOfItems(), numberOfItems);
    QCOMPARE(pagination.enableFirst(), false);
    QCOMPARE(pagination.enableLast(), true);

    QVector<int> pages = {1, 2, 3};
    qDebug() << pagination.pages();
    QCOMPARE(pagination.pages(), pages);
}

void TestPagination::testPaginationDisabledLastPageLink()
{
    int numberOfItems = 10;
    int itemsPerPage  = 2;
    int currentPage   = 10;
    int pageLinks     = 10;
    Pagination pagination(numberOfItems, itemsPerPage, currentPage, pageLinks);

    QCOMPARE(pagination.limit(), 2);
    QCOMPARE(pagination.offset(), 18);
    QCOMPARE(pagination.currentPage(), currentPage);
    QCOMPARE(pagination.lastPage(), 5);
    QCOMPARE(pagination.numberOfItems(), numberOfItems);
    QCOMPARE(pagination.enableFirst(), true);
    QCOMPARE(pagination.enableLast(), false);

    QVector<int> pages = {1, 2, 3, 4, 5};
    qDebug() << pagination.pages();
    QCOMPARE(pagination.pages(), pages);
}

QTEST_MAIN(TestPagination)
#include "testpagination.moc"

#endif
