#ifndef CHILDPROCESS_H
#define CHILDPROCESS_H

#include <QObject>
#include <QTcpServer>

class ChildProcess : public QObject
{
    Q_OBJECT
public:
    explicit ChildProcess(bool &childProcess, QObject *parent = 0);
    ~ChildProcess();

    bool initted() const;
    bool sendFD(int fd);

private:
    void initChild(int socket);
    void getFD(int socket);
    ssize_t sendFD(int sock, void *buf, ssize_t buflen, int fd);
    ssize_t readFD(int sock, void *buf, ssize_t bufsize, int *fd);

    QString m_error;
    int m_childFD;
    int m_parentFD;
    int m_childPID;
};

#endif // CHILDPROCESS_H
