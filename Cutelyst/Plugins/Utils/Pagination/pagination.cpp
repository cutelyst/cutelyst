/*
 * SPDX-FileCopyrightText: (C) 2015-2025 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "pagination.h"

#include <QtCore/QLoggingCategory>

using namespace Cutelyst;
using namespace Qt::StringLiterals;

Q_LOGGING_CATEGORY(C_PAGINATION, "cutelyst.utils.pagination", QtWarningMsg)

Pagination::Pagination(int numberOfItems, int itemsPerPage, int currentPage, int pageLinks)
{
    if (itemsPerPage <= 0) {
        qCWarning(C_PAGINATION) << "Invalid number of items per page:" << itemsPerPage
                                << "failing back to 1";
        itemsPerPage = 1;
    }

    if (currentPage <= 0) {
        qCWarning(C_PAGINATION) << "Invalid current page:" << currentPage << "failing back to 1";
        currentPage = 1;
    }

    if (pageLinks <= 0) {
        qCWarning(C_PAGINATION) << "Invalid number of page links:" << pageLinks
                                << "failing back to 1";
        pageLinks = 1;
    }

    insert(u"limit"_s, itemsPerPage);
    insert(u"offset"_s, (currentPage - 1) * itemsPerPage);
    insert(u"currentPage"_s, currentPage);
    insert(u"current"_s, currentPage);

    const int resultLastPage = ((numberOfItems - 1) / itemsPerPage) + 1;
    currentPage              = std::ranges::min(currentPage, resultLastPage);

    const int startPage = (currentPage < pageLinks + 1) ? 1 : currentPage - pageLinks;
    const int endPage   = std::ranges::min((pageLinks * 2) + startPage, resultLastPage);

    QVector<int> resultPages;
    for (int i = startPage; i <= endPage; ++i) {
        resultPages.append(i);
    }
    insert(u"enableFirst"_s, currentPage > 1);
    insert(u"enableLast"_s, currentPage != resultLastPage);
    insert(u"pages"_s, QVariant::fromValue(resultPages));
    insert(u"lastPage"_s, resultLastPage);
    insert(u"numberOfItems"_s, numberOfItems);
}

Pagination::~Pagination()
{
}

int Pagination::limit() const
{
    return value(u"limit"_s).toInt();
}

int Pagination::offset() const
{
    return value(u"offset"_s).toInt();
}

int Pagination::offset(int itemsPerPage, int currentPage)
{
    if (itemsPerPage <= 0) {
        qCWarning(C_PAGINATION) << "Invalid number of items per page:" << itemsPerPage
                                << "failing back to 1";
        itemsPerPage = 1;
    }
    if (currentPage <= 0) {
        qCWarning(C_PAGINATION) << "Invalid current page:" << currentPage << "failing back to 1";
        currentPage = 1;
    }
    return (currentPage - 1) * itemsPerPage;
}

int Pagination::currentPage() const
{
    return value(u"currentPage"_s).toInt();
}

int Pagination::lastPage() const
{
    return value(u"lastPage"_s).toInt();
}

int Pagination::numberOfItems() const
{
    return value(u"numberOfItems"_s).toInt();
}

bool Pagination::enableFirst() const
{
    return value(u"enableFirst"_s).toBool();
}

bool Pagination::enableLast() const
{
    return value(u"enableLast"_s).toBool();
}

QVector<int> Pagination::pages() const
{
    return value(u"pages"_s).value<QVector<int>>();
}

#include "moc_pagination.cpp"
