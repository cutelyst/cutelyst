/*
 * SPDX-FileCopyrightText: (C) 2025 Matthias Fehring <mf@huessenbergnetz.de>
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CUTELYST_VALIDATOR_CNAMERESOLVER_P_H
#define CUTELYST_VALIDATOR_CNAMERESOLVER_P_H

#include <QDnsHostAddressRecord>
#include <QDnsLookup>
#include <QObject>

namespace Cutelyst {

/**
 * \internal
 * \ingroup plugins-utils-validator
 * \brief DNS CNAME resolver
 *
 * This is a helper class to resolve A and AAAA records for domains that have a
 * CNAME ressource record.
 *
 * This is a fire and forget class. It will automatically be deleted when the finished
 * signal is emitted.
 *
 * <h3>Example usage</h3>
 * \code{.cpp}
 * auto cnameDns = new CnameResolver;
 * QObject::connect(cnameDns, &CnameResolver::finished, [](
 *                               const QList<QDnsHostAddressRecord> &hostRecords,
 *                               QDnsLookup::Error error) {
 *     if (error == QDnsLookup::NoError) {
 *         // handle hostRecords
 *     } else {
 *         // handle error
 *     }
 * });
 * cnameDns->start(QStringLiteral("cname.example.net"));
 * \endcode
 */
class CnameResolver : public QObject
{
    Q_OBJECT
public:
    /**
     * Constructs a new %CnameResolver object.
     */
    explicit CnameResolver(QObject *parent = nullptr);
    /**
     * Destructs the %CnameResolver object.
     */
    ~CnameResolver() override = default;

    /**
     * Starts the resolution of the \a cname record.
     *
     * The finished() signal will be emitted.
     */
    void start(const QString &cname);

private Q_SLOTS:
    void resolve(const QString &cname, QDnsLookup::Type type);

Q_SIGNALS:
    /**
     * Will be emitted if the resolution finishes.
     */
    void finished(const QList<QDnsHostAddressRecord> &hostRecords, QDnsLookup::Error error);

private:
    const int m_maxRecursion{5};
    int m_currentRun{1};
};

} // namespace Cutelyst

#endif // CUTELYST_VALIDATOR_CNAMERESOLVER_P_H
