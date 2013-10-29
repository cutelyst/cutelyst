#ifndef CUTELYSTCONNECTION_H
#define CUTELYSTCONNECTION_H

#include <QObject>

class CutelystRequest;
class CutelystConnectionPrivate;
class CutelystConnection : public QObject
{
    Q_OBJECT
public:
    explicit CutelystConnection(int socket, QObject *parent = 0);
    ~CutelystConnection();

    bool isValid() const;
    CutelystRequest* request() const;

protected:
    virtual void parse(const QByteArray &data) = 0;
    qint64 write(const QByteArray &data);
    CutelystConnectionPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystConnection)

    void readyRead();
};

#endif // CUTELYSTCONNECTION_H
