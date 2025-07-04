/*
 * SPDX-FileCopyrightText: (C) 2013-2022 Daniel Nicoletti <dantti12@gmail.com>
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "helper.h"

#include <QDir>
#include <QDirIterator>
#include <QMimeDatabase>

using namespace Qt::StringLiterals;

Helper::Helper(QObject *parent)
    : QObject(parent)
{
}

bool Helper::findProjectDir(const QDir &dir, QDir *projectDir)
{
    QFile cmake(dir.absoluteFilePath(u"CMakeLists.txt"_s));
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
        QString file         = it.next();
        const QMimeType mime = m_db.mimeTypeForFile(file);
        if (mime.inherits(u"application/x-sharedlib"_s)) {
            return file;
        }
    }
    return {};
}

#include "moc_helper.cpp"
