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

#include "transactionprivate.h"

#include "transactionproxy.h"

#include <QStringList>
#include <QDebug>

using namespace PackageKit;

TransactionPrivate::TransactionPrivate(Transaction* parent) :
    q_ptr(parent),
    p(0),
    role(Transaction::RoleUnknown)
{
}

void TransactionPrivate::Details(const QString &pid,
                                 const QString &license,
                                 uint group,
                                 const QString &detail,
                                 const QString &url,
                                 qulonglong size)
{
    Q_Q(Transaction);
    q->details(pid,
               license,
               static_cast<Transaction::Group>(group),
               detail,
               url,
               size);
}

void TransactionPrivate::distroUpgrade(uint type, const QString &name, const QString &description)
{
    Q_Q(Transaction);
    q->distroUpgrade(static_cast<Transaction::DistroUpgrade>(type),
                     name,
                     description);
}

void TransactionPrivate::errorCode(uint error, const QString &details)
{
    Q_Q(Transaction);
    q->errorCode(static_cast<Transaction::Error>(error), details);
}

void TransactionPrivate::mediaChangeRequired(uint mediaType, const QString &mediaId, const QString &mediaText)
{
    Q_Q(Transaction);
    q->mediaChangeRequired(static_cast<Transaction::MediaType>(mediaType),
                           mediaId,
                           mediaText);
}

void TransactionPrivate::finished(uint exitCode, uint runtime)
{
    Q_Q(Transaction);
    q->finished(static_cast<Transaction::Exit>(exitCode), runtime);
}

void TransactionPrivate::destroy()
{
    Q_Q(Transaction);
    if (p) {
       delete p;
       p = 0;
    }
    q->destroy();
}

void TransactionPrivate::daemonQuit()
{
    Q_Q(Transaction);
    if (p) {
        q->finished(Transaction::ExitFailed, 0);
        destroy();
    }
}

void TransactionPrivate::message(uint type, const QString &message)
{
    Q_Q(Transaction);
    q->message(static_cast<Transaction::Message>(type), message);
}

void TransactionPrivate::Package(uint info, const QString &pid, const QString &summary)
{
    Q_Q(Transaction);
    q->package(static_cast<Transaction::Info>(info),
               pid,
               summary);
}

void TransactionPrivate::ItemProgress(const QString &itemID, uint status, uint percentage)
{
    Q_Q(Transaction);
    q->itemProgress(itemID,
                    static_cast<PackageKit::Transaction::Status>(status),
                    percentage);
}

void TransactionPrivate::RepoSignatureRequired(const QString &pid,
                                               const QString &repoName,
                                               const QString &keyUrl,
                                               const QString &keyUserid,
                                               const QString &keyId,
                                               const QString &keyFingerprint,
                                               const QString &keyTimestamp,
                                               uint type)
{
    Q_Q(Transaction);
    q->repoSignatureRequired(pid,
                             repoName,
                             keyUrl,
                             keyUserid,
                             keyId,
                             keyFingerprint,
                             keyTimestamp,
                             static_cast<Transaction::SigType>(type));
}

void TransactionPrivate::requireRestart(uint type, const QString &pid)
{
    Q_Q(Transaction);
    q->requireRestart(static_cast<PackageKit::Transaction::Restart>(type), pid);
}

void TransactionPrivate::transaction(const QDBusObjectPath &oldTid,
                                     const QString &timespec,
                                     bool succeeded,
                                     uint role,
                                     uint duration,
                                     const QString &data,
                                     uint uid,
                                     const QString &cmdline)
{
    Q_Q(Transaction);
    q->transaction(new Transaction(oldTid, timespec, succeeded, static_cast<Transaction::Role>(role), duration, data, uid, cmdline, q->parent()));
}

void TransactionPrivate::UpdateDetail(const QString &package_id,
                                      const QStringList &updates,
                                      const QStringList &obsoletes,
                                      const QStringList &vendor_urls,
                                      const QStringList &bugzilla_urls,
                                      const QStringList &cve_urls,
                                      uint restart,
                                      const QString &update_text,
                                      const QString &changelog,
                                      uint state,
                                      const QString &issued,
                                      const QString &updated)
{
    Q_Q(Transaction);
    q->updateDetail(package_id,
                    updates,
                    obsoletes,
                    vendor_urls,
                    bugzilla_urls,
                    cve_urls,
                    static_cast<PackageKit::Transaction::Restart>(restart),
                    update_text,
                    changelog,
                    static_cast<PackageKit::Transaction::UpdateState>(state),
                    QDateTime::fromString(issued, Qt::ISODate),
                    QDateTime::fromString(updated, Qt::ISODate));
}
