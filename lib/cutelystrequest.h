#ifndef CUTELYSTREQUEST_H
#define CUTELYSTREQUEST_H

#include <QObject>
#include <QTcpSocket>

class CutelystRequest : public QObject
{
    Q_OBJECT
public:
    explicit CutelystRequest(QObject *parent = 0);
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
    QVariant cookie(const QString &name) const;
    QVariantHash cookies() const;
    QVariant header(const QString &name) const;
    QVariantHash headers() const;
    QString method() const;
    QString protocol() const;
    QString userAgent() const;
};

#endif // CUTELYSTREQUEST_H
