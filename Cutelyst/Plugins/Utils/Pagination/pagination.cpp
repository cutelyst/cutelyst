/*
 * Copyright (C) 2015-2017 Daniel Nicoletti <dantti12@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "pagination.h"

#include <QtCore/QLoggingCategory>

using namespace Cutelyst;

Q_LOGGING_CATEGORY(C_PAGINATION, "cutelyst.utils.pagination")

Pagination::Pagination()
{

}

Pagination::Pagination(int numberOfItems, int itemsPerPage, int currentPage, int pageLinks)
{
    if (itemsPerPage <= 0) {
        qCWarning(C_PAGINATION) << "Invalid number of items per page:" << itemsPerPage << "failing back to 1";
        itemsPerPage = 1;
    }

    if (currentPage <= 0) {
        qCWarning(C_PAGINATION) << "Invalid current page:" << currentPage  << "failing back to 1";
        currentPage = 1;
    }

    if (pageLinks <= 0) {
        qCWarning(C_PAGINATION) << "Invalid number of page links:" << pageLinks << "failing back to 1";
        pageLinks = 1;
    }

    insert(QStringLiteral("limit"), itemsPerPage);
    insert(QStringLiteral("offset"), (currentPage - 1) * itemsPerPage);
    insert(QStringLiteral("currentPage"), currentPage);
    insert(QStringLiteral("current"), currentPage);

    int lastPage = (numberOfItems - 1) / itemsPerPage + 1;
    if (currentPage > lastPage) {
        currentPage = lastPage;
    }

    int startPage = (currentPage < pageLinks + 1) ? 1 : currentPage - pageLinks;
    int endPage = (pageLinks * 2) + startPage;
    if (lastPage < endPage) {
        endPage = lastPage;
    }

    QVector<int> pages;
    for (int i = startPage; i <= endPage; ++i) {
        pages.append(i);
    }
    insert(QStringLiteral("enableFirst"), currentPage > 1);
    insert(QStringLiteral("enableLast"), currentPage != lastPage);
    insert(QStringLiteral("pages"), QVariant::fromValue(pages));
    insert(QStringLiteral("lastPage"), lastPage);
    insert(QStringLiteral("numberOfItems"), numberOfItems);
}

Pagination::~Pagination()
{

}

int Pagination::limit() const
{
    return value(QStringLiteral("limit")).toInt();
}

int Pagination::offset() const
{
    return value(QStringLiteral("offset")).toInt();
}

int Pagination::currentPage() const
{
    return value(QStringLiteral("currentPage")).toInt();
}

int Pagination::lastPage() const
{
    return value(QStringLiteral("lastPage")).toInt();
}

int Pagination::numberOfItems() const
{
    return value(QStringLiteral("numberOfItems")).toInt();
}

bool Pagination::enableFirst() const
{
    return value(QStringLiteral("enableFirst")).toBool();
}

bool Pagination::enableLast() const
{
    return value(QStringLiteral("enableLast")).toBool();
}

QVector<int> Pagination::pages() const
{
    return value(QStringLiteral("pages")).value<QVector<int> >();
}
