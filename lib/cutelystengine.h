#ifndef CUTELYSTENGINE_H
#define CUTELYSTENGINE_H

#include <QObject>

class CutelystEnginePrivate;
class CutelystEngine : public QObject
{
    Q_OBJECT
public:
    explicit CutelystEngine(bool &childProcess, QObject *parent = 0);
    ~CutelystEngine();

    bool initted() const;
    bool sendFD(int fd);

protected:
    CutelystEnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystEngine)

    void gotFD(int socket);
};

#endif // CUTELYSTENGINE_H
