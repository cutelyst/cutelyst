/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "helper.h"

#include <QMimeDatabase>
#include <QDir>
#include <QDirIterator>

Helper::Helper(QObject *parent) : QObject(parent)
{

}

bool Helper::findProjectDir(const QDir &dir, QDir *projectDir)
{
    QFile cmake(dir.absoluteFilePath(QStringLiteral("CMakeLists.txt")));
    if (cmake.exists()) {
        if (cmake.open(QFile::ReadOnly | QFile::Text)) {
            while (!cmake.atEnd()) {
                QByteArray line = cmake.readLine();
                if (line.toLower().startsWith(QByteArrayLiteral("project"))) {
                    *projectDir = dir;
                    return true;
                }
            }
        }
    }

    QDir localDir = dir;
    if (localDir.cdUp()) {
        return findProjectDir(localDir, projectDir);
    }
    return false;
}

QString Helper::findApplication(const QDir &projectDir)
{
    QMimeDatabase m_db;

    QDirIterator it(projectDir.absolutePath(), QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString file = it.next();
        const QMimeType mime = m_db.mimeTypeForFile(file);
        if (mime.inherits(QStringLiteral("application/x-sharedlib"))) {
            return file;
        }
    }
    return QString();
}

#include "moc_helper.cpp"
