#ifndef HELPER_H
#define HELPER_H

#include <QObject>

class QDir;
class Helper : public QObject
{
    Q_OBJECT
public:
    explicit Helper(QObject *parent = 0);

    static bool findProjectDir(const QDir &dir, QDir *projectDir);
    static QString findApplication(const QDir &projectDir);
};

#endif // HELPER_H
