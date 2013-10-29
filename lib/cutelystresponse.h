#ifndef CUTELYSTRESPONSE_H
#define CUTELYSTRESPONSE_H

#include <QObject>

class CutelystResponse : public QObject
{
    Q_OBJECT
public:
    explicit CutelystResponse(QObject *parent = 0);

    void setBody(const QByteArray &body);
    void setContentEncoding(const QString &encoding);
    void setContentLength(quint64 lenght);
    void setContentType(const QString &encoding);
    void setCookie(const QString &key, const QString &value);
    void setCookies(const QHash<QString, QString> &cookies);

    void redirect(const QUrl &url, quint16 status = 302);
    void setLocation(const QString &location);
    void setStatus(quint64 status);
    void write(const QByteArray &data);
};

#endif // CUTELYSTRESPONSE_H
