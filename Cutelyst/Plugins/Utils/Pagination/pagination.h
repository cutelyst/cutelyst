/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PAGINATION_H
#define PAGINATION_H

#include <Cutelyst/cutelyst_global.h>

#include <QVariantMap>

namespace Cutelyst {

class CUTELYST_PLUGIN_UTILS_PAGINATION_EXPORT Pagination : public QVariantMap
{
    Q_GADGET
    Q_PROPERTY(int limit READ limit CONSTANT)
    Q_PROPERTY(int offset READ offset CONSTANT)
    Q_PROPERTY(int currentPage READ currentPage CONSTANT)
    Q_PROPERTY(int lastPage READ lastPage CONSTANT)
    Q_PROPERTY(int numberOfItems READ numberOfItems CONSTANT)
    Q_PROPERTY(bool enableFirst READ enableFirst CONSTANT)
    Q_PROPERTY(bool enableLast READ enableLast CONSTANT)
    Q_PROPERTY(QVector<int> pages READ pages CONSTANT)
public:
    Pagination() = default;
    /**
     * Contructs a pagination object
     * @param numberOfItems should be set to the total of items to be displayed
     * @param itemsPerPage the desired number of items per page
     * @param currentPage the current 1 indexed page (first page is 1) usually from the query url
     * @param pageLinks the number of page links that should be generated, for example 3 -> 11, 12, 13
     */
    Pagination(int numberOfItems, int itemsPerPage, int currentPage, int pageLinks = 10);
    virtual ~Pagination();

    /**
     * Returns the number os items per page
     */
    int limit() const;

    /**
     * Returns the current page offset for use in SQL
     */
    int offset() const;

    /**
     * Returns the current page offset for use in SQL taking the number of items per page and the current page
     */
    static int offset(int itemsPerPage, int currentPage);

    /**
     * Returns the current page number
     */
    int currentPage() const;

    /**
     * Returns the number of the last page
     */
    int lastPage() const;

    /**
     * Returns the total number of items
     */
    int numberOfItems() const;

    /**
     * Returns true if the first page link should be enabled
     */
    bool enableFirst() const;

    /**
     * Returns true if the last page link should be enabled
     */
    bool enableLast() const;

    /**
     * Returns the list of pages, with each page number as the integer
     */
    QVector<int> pages() const;
};

} // namespace Cutelyst
Q_DECLARE_METATYPE(Cutelyst::Pagination)

#endif // PAGINATION_H
