#ifndef CUTELYSTENGINE_H
#define CUTELYSTENGINE_H

#include <QObject>
#include <QHostAddress>

class CutelystRequest;
class CutelystEnginePrivate;
class CutelystEngine : public QObject
{
    Q_OBJECT
public:
    explicit CutelystEngine(int socket, QObject *parent = 0);
    ~CutelystEngine();

    bool isValid() const;
    CutelystRequest* request() const;
    quint16 peerPort() const;
    QString peerName() const;
    QHostAddress peerAddress() const;

protected:
    virtual void parse(const QByteArray &data) = 0;
    qint64 write(const QByteArray &data);
    CutelystEnginePrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystEngine)

    void readyRead();
};

#endif // CUTELYSTENGINE_H
