#include "helper.h"

#include <QMimeDatabase>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

Helper::Helper(QObject *parent) : QObject(parent)
{

}

bool Helper::run(const QString &appFilename, int port, bool restart)
{
    QDir projectDir;
    if (!Helper::findProjectDir(QDir::current(), &projectDir)) {
        qDebug() << "Error: failed to find project";
        return false;
    }

    QString localFilename = appFilename;
    if (localFilename.isEmpty()) {
        localFilename = findApplication(projectDir);
    }

    QFileInfo fileInfo(localFilename);
    if (!fileInfo.exists()) {
        qDebug() << "Error: Application file not found";
        return false;
    }

    QStringList args;
    args.append(QStringLiteral("--http-socket"));
    args.append(QLatin1String(":") % QString::number(port));

    args.append(QStringLiteral("--chdir"));
    args.append(projectDir.absolutePath());

    args.append(QStringLiteral("-M"));

    args.append(QStringLiteral("--plugin"));
    args.append(QStringLiteral("cutelyst"));

    args.append(QStringLiteral("--cutelyst-app"));
    args.append(localFilename);

    if (restart) {
        args.append(QStringLiteral("--lazy"));
        args.append(QStringLiteral("--touch-reload"));
        args.append(localFilename);
    }

    qDebug() << "Running: uwsgi" << args.join(QStringLiteral(" ")).toLatin1().data();

//    // Enable loggin if restart is set
//    if (restart && !qEnvironmentVariableIsSet("QT_LOGGING_RULES")) {
//        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
//        env.insert(QStringLiteral("QT_LOGGING_RULES"), QStringLiteral("cutelyst.*=true"));
//        m_proc->setProcessEnvironment(env);
//    }


    return true;
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
        if (mime.inherits(QStringLiteral("application/x-sharedlib")) ||
                mime.inherits(QStringLiteral("application/x-mach-binary"))) {
            return file;
        }
    }
    return QString();
}
