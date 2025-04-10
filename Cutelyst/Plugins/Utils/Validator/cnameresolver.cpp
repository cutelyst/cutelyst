/*
 * SPDX-FileCopyrightText: (C) 2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "cnameresolver_p.h"

#include <chrono>

#include <QDebug>
#include <QTimer>

using namespace Cutelyst;

CnameResolver::CnameResolver(QObject *parent)
    : QObject{parent}
{
    connect(this, &CnameResolver::finished, this, &QObject::deleteLater);
}

void CnameResolver::start(const QString &cname)
{
    QMetaObject::invokeMethod(this, "resolve", cname, QDnsLookup::CNAME);
}

void CnameResolver::resolve(const QString &cname, QDnsLookup::Type type)
{
    if (m_currentRun <= m_maxRecursion) {
        ++m_currentRun;
        auto dns = new QDnsLookup{type, cname};
        connect(dns, &QDnsLookup::finished, this, [dns, type, cname, this] {
            if (dns->error() == QDnsLookup::NoError) {
                if (type == QDnsLookup::CNAME) {
                    if (dns->canonicalNameRecords().empty()) {
                        resolve(cname, QDnsLookup::ANY);
                    } else {
                        const auto cnameRecords = dns->canonicalNameRecords();
                        resolve(cnameRecords.last().value(), QDnsLookup::CNAME);
                    }
                } else {
                    if (dns->hostAddressRecords().empty()) {
                        Q_EMIT finished({}, QDnsLookup::NotFoundError);
                    } else {
                        Q_EMIT finished(dns->hostAddressRecords(), QDnsLookup::NoError);
                    }
                }
            } else {
                Q_EMIT finished({}, dns->error());
            }

            dns->deleteLater();
        });
        QTimer::singleShot(std::chrono::seconds{10}, dns, &QDnsLookup::abort);
        dns->lookup();
    } else {
        Q_EMIT finished({}, QDnsLookup::OperationCancelledError);
    }
}
