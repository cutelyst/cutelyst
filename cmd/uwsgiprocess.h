#ifndef UWSGIPROCESS_H
#define UWSGIPROCESS_H

#include <QObject>

class QDir;
class QProcess;
class QSocketNotifier;
class uwsgiProcess : public QObject
{
    Q_OBJECT
public:
    explicit uwsgiProcess(QObject *parent = 0);

#ifdef Q_OS_UNIX
    // Unix signal handlers.
    static void hupSignalHandler(int unused);
    static void termSignalHandler(int unused);
    static void killSignalHandler(int unused);
    static void intSignalHandler(int unused);

    int setup_unix_signal_handlers();
#endif

    bool run(const QString &appFilename, int port, bool restart);

    static bool findProjectDir(const QDir &dir, QDir *projectDir);
    static QString findApplication(const QDir &projectDir);

public Q_SLOTS:
    // Qt signal handlers.
#ifdef Q_OS_UNIX
    void handleSigHup();
    void handleSigTerm();
    void handleSigKill();
    void handleSigInt();
#endif
    void processFinished(int exitCode);

private:
    QProcess *m_proc;
};

#endif // UWSGIPROCESS_H
