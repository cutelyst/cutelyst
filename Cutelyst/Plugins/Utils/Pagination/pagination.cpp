#include "pagination.h"

using namespace Cutelyst::Utils;

QVariantMap Pagination::page(int numberOfItems, int itemsPerPage, int currentPage, int pageLinks)
{
    QVariantMap ret;

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
