/*
 * SPDX-FileCopyrightText: (C) 2015-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef PAGINATION_H
#define PAGINATION_H

#include <Cutelyst/cutelyst_global.h>

#include <QVariantMap>

namespace Cutelyst {

/**
 * @ingroup plugins-utils
 * @headerfile "" <Cutelyst/Plugins/Utils/Pagination>
 * @brief Helper to calculate values for paginating result lists.
 *
 * The %Pagination plugin is a little helper class to calculate different values for paginating
 * result lists from for example database queries. If you have a result set of for example 123
 * records and want to show 10 per page, you can use this class to calculate the pagination.
 *
 * \logcat{utils.pagination}
 */
class CUTELYST_PLUGIN_UTILS_PAGINATION_EXPORT Pagination : public QVariantMap
{
    Q_GADGET
    /**
     * Returns the number os items per page.
     */
    Q_PROPERTY(int limit READ limit CONSTANT)
    /**
     * Returns the current page offset for use in SQL.
     */
    Q_PROPERTY(int offset READ offset CONSTANT)
    /**
     * Returns the current page number.
     */
    Q_PROPERTY(int currentPage READ currentPage CONSTANT)
    /**
     * Returns the number of the last page.
     */
    Q_PROPERTY(int lastPage READ lastPage CONSTANT)
    /**
     * Returns the total number of items.
     */
    Q_PROPERTY(int numberOfItems READ numberOfItems CONSTANT)
    /**
     * Returns @c true if the first page link should be enabled.
     */
    Q_PROPERTY(bool enableFirst READ enableFirst CONSTANT)
    /**
     * Returns @c true if the last page link should be enabled.
     */
    Q_PROPERTY(bool enableLast READ enableLast CONSTANT)
    /**
     * Returns the list of pages, with each page number as the integer.
     */
    Q_PROPERTY(QVector<int> pages READ pages CONSTANT)
public:
    Pagination() = default;
    /**
     * Contructs a new %Pagination object with the given parameters.
     * @param numberOfItems should be set to the total of items to be displayed
     * @param itemsPerPage the desired number of items per page
     * @param currentPage the current 1 indexed page (first page is 1) usually from the query url
     * @param pageLinks the number of page links that should be generated, for example 3 -> 11, 12,
     * 13
     */
    Pagination(int numberOfItems, int itemsPerPage, int currentPage, int pageLinks = 10);

    /**
     * Destroys the %Pagination object.
     */
    virtual ~Pagination();

    /**
     * Returns the number os items per page.
     */
    [[nodiscard]] int limit() const;

    /**
     * Returns the current page offset for use in SQL.
     */
    [[nodiscard]] int offset() const;

    /**
     * Returns the current page offset for use in SQL taking the number of items per page and the
     * current page.
     */
    [[nodiscard]] static int offset(int itemsPerPage, int currentPage);

    /**
     * Returns the current page number.
     */
    [[nodiscard]] int currentPage() const;

    /**
     * Returns the number of the last page.
     */
    [[nodiscard]] int lastPage() const;

    /**
     * Returns the total number of items.
     */
    [[nodiscard]] int numberOfItems() const;

    /**
     * Returns @c true if the first page link should be enabled.
     */
    bool enableFirst() const;

    /**
     * Returns @c true if the last page link should be enabled.
     */
    bool enableLast() const;

    /**
     * Returns the list of pages, with each page number as the integer.
     */
    [[nodiscard]] QVector<int> pages() const;
};

} // namespace Cutelyst
Q_DECLARE_METATYPE(Cutelyst::Pagination)

#endif // PAGINATION_H
