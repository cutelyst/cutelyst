/*
 * SPDX-FileCopyrightText: (C) 2017-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "urifor.h"

#include <Context>
#include <Cutelyst/Context>
#include <Cutelyst/ParamsMultiMap>
#include <cutelee/exception.h>
#include <cutelee/parser.h>

#include <QDebug>

UriFor::UriFor(const QString &path, const QStringList &args, Cutelee::Parser *parser)
    : Cutelee::Node(parser)
    , m_path(path, parser)
{
    bool foundQuery = false;
    for (const QString &expression : args) {
        // WE require the QUERY keyword to know when we are dealing with query values
        if (expression.compare(u"QUERY") == 0) {
            foundQuery = true;
            continue;
        }

        if (foundQuery) {
            m_queryExpressions.push_back(Cutelee::FilterExpression(expression, parser));
        } else {
            m_argsExpressions.push_back(Cutelee::FilterExpression(expression, parser));
        }
    }
    std::reverse(m_queryExpressions.begin(), m_queryExpressions.end());
}

std::pair<QString, QString> splitQuery(const QString &query)
{
    std::pair<QString, QString> ret;

    ret.first  = query.section(QLatin1Char('='), 0, 0);
    ret.second = query.section(QLatin1Char('='), 1);

    return ret;
}

void UriFor::render(Cutelee::OutputStream *stream, Cutelee::Context *gc) const
{
    // In case cutelyst context is not set as "c"
    auto c = gc->lookup(m_cutelystContext).value<Cutelyst::Context *>();
    if (!c) {
        const QVariantHash hash = gc->stackHash(0);
        auto it                 = hash.constBegin();
        while (it != hash.constEnd()) {
            if (it.value().userType() == qMetaTypeId<Cutelyst::Context *>()) {
                c = it.value().value<Cutelyst::Context *>();
                if (c) {
                    m_cutelystContext = it.key();
                    break;
                }
            }
            ++it;
        }

        if (!c) {
            return;
        }
    }

    QString path;
    QStringList args;
    Cutelyst::ParamsMultiMap queryValues;

    QVariant pathVar = m_path.resolve(gc);
    if (pathVar.userType() == qMetaTypeId<Cutelee::SafeString>()) {
        path = pathVar.value<Cutelee::SafeString>().get();
    } else if (pathVar.typeId() == QMetaType::QString) {
        path = pathVar.toString();
    } else {
        qWarning() << "c_uri_for PATH is not a valid type";
        return;
    }

    for (const Cutelee::FilterExpression &exp : m_argsExpressions) {
        QVariant var = exp.resolve(gc);
        if (var.userType() == qMetaTypeId<Cutelee::SafeString>()) {
            args << var.value<Cutelee::SafeString>().get();
        } else if (var.typeId() == QMetaType::QString) {
            args << var.toString();
        } else if (var.typeId() == QMetaType::QStringList) {
            args << var.toStringList();
        }
    }

    for (const Cutelee::FilterExpression &exp : m_queryExpressions) {
        QVariant var = exp.resolve(gc);
        if (var.userType() == qMetaTypeId<Cutelyst::ParamsMultiMap>()) {
            auto map = var.value<Cutelyst::ParamsMultiMap>();
            queryValues.unite(map);
        } else if (var.userType() == qMetaTypeId<Cutelee::SafeString>()) {
            auto query = splitQuery(var.value<Cutelee::SafeString>().get());
            queryValues.insert(query.first, query.second);
        } else if (var.typeId() == QMetaType::QString) {
            auto query = splitQuery(var.toString());
            queryValues.insert(query.first, query.second);
        } else if (var.typeId() == QMetaType::QStringList) {
            const auto queries = var.toStringList();
            for (const QString &str : queries) {
                auto query = splitQuery(str);
                queryValues.insert(query.first, query.second);
            }
        }
    }

    *stream << c->uriFor(path, args, queryValues).toString(QUrl::FullyEncoded);
}

Cutelee::Node *UriForTag::getNode(const QString &tagContent, Cutelee::Parser *p) const
{
    // You almost always want to use smartSplit.
    QStringList parts = smartSplit(tagContent);

    parts.removeFirst(); // Not interested in the name of the tag.
    if (parts.isEmpty()) {
        throw Cutelee::Exception(Cutelee::TagSyntaxError,
                                 QStringLiteral("c_uri_for requires at least the path"));
    }

    return new UriFor(parts.first(), parts.mid(1), p);
}

#include "moc_urifor.cpp"
