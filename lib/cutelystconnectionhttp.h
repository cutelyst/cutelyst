#ifndef CUTELYSTCONNECTIONHTTP_H
#define CUTELYSTCONNECTIONHTTP_H

#include "cutelystconnection.h"

#include <QTcpSocket>
#include <QStringList>

class CutelystConnectionHttp : public CutelystConnection
{
    Q_OBJECT
public:
    explicit CutelystConnectionHttp(int socket, QObject *parent = 0);

    virtual QStringList args() const;
    virtual QString base() const;
    virtual QString body() const;
    virtual QVariantHash bodyParameters() const;
    virtual QString contentEncoding() const;
    virtual QVariantHash cookies() const;
    virtual QVariantHash headers() const;
    virtual QString method() const;
    virtual QString protocol() const;
    virtual QString userAgent() const;

protected:
    virtual void parse(const QByteArray &request);

private:
    QVariantHash m_data;
    QByteArray m_buffer;
    quint64 m_bufLastIndex;
    QString m_method;
    QStringList m_args;
    QString m_protocol;
    QHash<QString, QString> m_headers;
};

#endif // CUTELYSTCONNECTIONHTTP_H
