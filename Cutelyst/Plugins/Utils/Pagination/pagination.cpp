#include "pagination.h"

#include <QtCore/QLoggingCategory>

using namespace Cutelyst::Utils;

Q_LOGGING_CATEGORY(C_PAGINATION, "cutelyst.utils.pagination")

QVariantMap Pagination::page(int numberOfItems, int itemsPerPage, int currentPage, int pageLinks)
{
    QVariantMap ret;

    if (itemsPerPage <= 0) {
        qCWarning(C_PAGINATION) << "Invalid number of items per page:" << itemsPerPage << "failing back to 10";
        itemsPerPage = 10;
    }

    if (currentPage < 0) {
        qCWarning(C_PAGINATION) << "Invalid current page:" << currentPage  << "failing back to 0";
        currentPage = 0;
    }

    if (pageLinks < 0) {
        qCWarning(C_PAGINATION) << "Invalid number of page links:" << pageLinks << "failing back to 10";
        pageLinks = 10;
    }

    ret.insert(QStringLiteral("limit"), itemsPerPage);
    ret.insert(QStringLiteral("current"), currentPage);
    ret.insert(QStringLiteral("sqlpage"), (currentPage - 1) * itemsPerPage);

    int lastPage = (numberOfItems - 1) / itemsPerPage + 1;
    if (currentPage > lastPage) {
        currentPage = lastPage;
    }

    int startPage = (currentPage < pageLinks + 1) ? 1 : currentPage - pageLinks;
    int endPage = (pageLinks * 2) + startPage;
    if (lastPage < endPage) {
        endPage = lastPage;
    }

    QList<int> pages;
    for (int i = startPage; i <= endPage; ++i) {
        pages.append(i);
    }
    ret.insert(QStringLiteral("enable_first"), currentPage > 1);
    ret.insert(QStringLiteral("enable_last"), currentPage < lastPage);
    ret.insert(QStringLiteral("pages"), QVariant::fromValue(pages));
    ret.insert(QStringLiteral("last_page"), lastPage);

    return ret;
}
