/*
 * Copyright (C) 2015 Daniel Nicoletti <dantti12@gmail.com>
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

#ifndef CSQL_H
#define CSQL_H

#include <QtCore/QVariant>
#include <QtSql/QSqlDatabase>

#include <Cutelyst/cutelyst_global.h>
#include <Cutelyst/paramsmultimap.h>

namespace Cutelyst {

namespace Sql {

    /**
     * Returns a QVariant hash for the first (if any) row
     * in the query object, it's useful for creating
     * stash objects for say an user
     */
    QVariantHash queryToHashObject(QSqlQuery &query);

    /**
     * Returns a variant list of QVariant hashes for all the rows
     * in the query object, it's useful for creating
     * stash objects for say a list of users
     */
    QVariantList queryToHashList(QSqlQuery &query);

    /**
     * Returns a QVariant map for the first (if any) row
     * in the query object, it's useful for creating
     * stash objects for say an user
     */
    QVariantMap queryToMapObject(QSqlQuery &query);

    /**
     * Returns a variant list of QVariant maps for all the rows
     * in the query object, it's useful for creating
     * stash objects for say a list of users to be used by
     * JSON serializer
     */
    QVariantList queryToMapList(QSqlQuery &query);

    /**
     * Bind params to the query, using the param name as
     * the placeholder prebended with ':', if htmlEscaped
     * is true the bound values will be the return of toHtmlEscaped()
     */
    void bindParamsToQuery(QSqlQuery &query, const Cutelyst::ParamsMultiMap &params, bool htmlEscaped = true);

    /**
     * Returns a QSqlQuery object prepared with \pa query using the \pa db database
     * This is specially useful to avoid pointers to prepered queries.
     * Best used as
     * static QSqlQuery query = preparedQuery("SELECT * FROM", someDb);
     */
    QSqlQuery preparedQuery(const QString &query, QSqlDatabase db = QSqlDatabase());

}

}

#endif // CSESSION_H
