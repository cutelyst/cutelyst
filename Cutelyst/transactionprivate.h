/*
 * This file is part of the QPackageKit project
 * Copyright (C) 2008 Adrien Bustany <madcat@mymadcat.com>
 * Copyright (C) 2010-2011 Daniel Nicoletti <dantti85-pk@yahoo.com.br>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB. If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef PACKAGEKIT_TRANSACTION_PRIVATE_H
#define PACKAGEKIT_TRANSACTION_PRIVATE_H

#include <QString>
#include <QHash>
#include <QStringList>

#include "transaction.h"

class TransactionProxy;

namespace PackageKit {

class TransactionPrivate
{
    Q_DECLARE_PUBLIC(Transaction)
protected:
    TransactionPrivate(Transaction *parent);
    virtual ~TransactionPrivate() {}

    QDBusObjectPath tid;
    ::TransactionProxy* p;
    Transaction *q_ptr;
    QStringList connectedSignals;

    // Only used for old transactions
    QDateTime timespec;
    Transaction::Role role;
    bool succeeded;
    uint duration;
    QString data;
    uint uid;
    QString cmdline;

    Transaction::InternalError error;
    QString errorMessage;

    void setupSignal(const QString &signal, bool connect);

protected Q_SLOTS:
    void Details(const QString &pid, const QString &license, uint group, const QString &detail, const QString &url, qulonglong size);
    void distroUpgrade(uint type, const QString &name, const QString &description);
    void errorCode(uint error, const QString &details);
    void mediaChangeRequired(uint mediaType, const QString &mediaId, const QString &mediaText);
    void finished(uint exitCode, uint runtime);
    void message(uint type, const QString &message);
    void Package(uint info, const QString &pid, const QString &summary);
    void ItemProgress(const QString &itemID, uint status, uint percentage);
    void RepoSignatureRequired(const QString &pid,
                               const QString &repoName,
                               const QString &keyUrl,
                               const QString &keyUserid,
                               const QString &keyId,
                               const QString &keyFingerprint,
                               const QString &keyTimestamp,
                               uint type);
    void requireRestart(uint type, const QString &pid);
    void transaction(const QDBusObjectPath &oldTid, const QString &timespec, bool succeeded, uint role, uint duration, const QString &data, uint uid, const QString &cmdline);
    void UpdateDetail(const QString &package_id, const QStringList &updates, const QStringList &obsoletes, const QStringList &vendor_urls, const QStringList &bugzilla_urls, const QStringList &cve_urls, uint restart, const QString &update_text, const QString &changelog, uint state, const QString &issued, const QString &updated);
    void destroy();
    void daemonQuit();
};

} // End namespace PackageKit

#endif
