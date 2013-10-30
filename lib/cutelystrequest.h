#ifndef CUTELYSTREQUEST_H
#define CUTELYSTREQUEST_H

#include <QObject>
#include <QTcpSocket>

class CutelystRequestPrivate;
class CutelystRequest
{
public:
    CutelystRequest();
    CutelystRequest(CutelystRequestPrivate *prv);
    ~CutelystRequest();

    /**
     * @brief peerAddress
     * @return the address of the client
     */
    QHostAddress peerAddress() const;

    /**
     * @brief peerName
     * @return the hostname of the client
     */
    QString peerName() const;

    /**
     * @brief peerPort
     * @return the originating port of the client
     */
    quint16 peerPort() const;

    QStringList args() const;
    QString base() const;
    QString body() const;
    QVariantHash bodyParameters() const;
    QString contentEncoding() const;
    QString cookie(const QString &key) const;
    QHash<QString, QString> cookies() const;
    QString header(const QString &key) const;
    QHash<QString, QString> headers() const;
    QString method() const;
    QString protocol() const;
    QString userAgent() const;

protected:
    CutelystRequestPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(CutelystRequest)
};

#endif // CUTELYSTREQUEST_H
