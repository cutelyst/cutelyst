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
#ifndef PAGINATION_H
#define PAGINATION_H

#include <QVariantMap>
#include <Cutelyst/cutelyst_global.h>

namespace Cutelyst {

class CUTELYST_PLUGIN_UTILS_PAGINATION_EXPORT Pagination : public QVariantMap
{
    Q_GADGET
    Q_PROPERTY(int limit READ limit)
    Q_PROPERTY(int offset READ offset)
    Q_PROPERTY(int currentPage READ currentPage)
    Q_PROPERTY(int lastPage READ lastPage)
    Q_PROPERTY(int numberOfItems READ numberOfItems)
    Q_PROPERTY(bool enableFirst READ enableFirst)
    Q_PROPERTY(bool enableLast READ enableLast)
    Q_PROPERTY(QVector<int> pages READ pages)
public:
    Pagination();
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

}
Q_DECLARE_METATYPE(Cutelyst::Pagination)

#endif // PAGINATION_H
