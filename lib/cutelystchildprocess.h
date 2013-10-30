#ifndef CUTELYSTCHILDPROCESS_H
#define CUTELYSTCHILDPROCESS_H

#include <QObject>

class CutelystChildProcessPrivate;
class CutelystChildProcess : public QObject
{
    Q_OBJECT
public:
    explicit CutelystChildProcess(bool &childProcess, QObject *parent = 0);
    ~CutelystChildProcess();

    bool initted() const;
    bool sendFD(int fd);

protected:
    CutelystChildProcessPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystChildProcess)

    void initChild(int socket);
    void gotFD(int socket);
};

#endif // CUTELYSTCHILDPROCESS_H
