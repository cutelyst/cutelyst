#include "cutelystenginehttp.h"
#include "cutelystrequest_p.h"

#include <QStringList>

CutelystEngineHttp::CutelystEngineHttp(int socket, QObject *parent) :
    CutelystEngine(socket, parent),
    m_bufLastIndex(0)
{
}

QStringList CutelystEngineHttp::args() const
{

}

QString CutelystEngineHttp::base() const
{

}

QString CutelystEngineHttp::body() const
{

}

QVariantHash CutelystEngineHttp::bodyParameters() const
{

}

QString CutelystEngineHttp::contentEncoding() const
{

}

QVariantHash CutelystEngineHttp::cookies() const
{

}

QVariantHash CutelystEngineHttp::headers() const
{

}

QString CutelystEngineHttp::method() const
{

}

QString CutelystEngineHttp::protocol() const
{

}

QString CutelystEngineHttp::userAgent() const
{
    return m_headers.value(QLatin1String("User-Agent"));
}

void CutelystEngineHttp::parse(const QByteArray &request)
{
    m_buffer.prepend(request);

    int newLine;
    if (m_method.isEmpty()) {
        if ((newLine = request.indexOf('\n', m_bufLastIndex)) != -1) {
            QByteArray section = request.mid(m_bufLastIndex, newLine - m_bufLastIndex - 1);
            m_bufLastIndex = newLine + 1;

            QRegExp methodProtocolRE("(\\w+)\\s+(.*)(?:\\s+(HTTP.*))$");
            QRegExp methodRE("(\\w+)\\s+(.*)");
            bool badRequest = false;
            if (methodProtocolRE.indexIn(section) != -1) {
                qDebug() << 1 <<  methodProtocolRE.capturedTexts() << methodProtocolRE.captureCount();
                if (methodProtocolRE.captureCount() == 3) {
                    m_method = methodProtocolRE.cap(1);
                    m_args = methodProtocolRE.cap(2).split(QLatin1String("/"));
                    m_protocol = methodProtocolRE.cap(3);
                } else {
                    badRequest = true;
                }
            } else if (methodRE.indexIn(section) != -1) {
                qDebug() << 2 << methodRE.capturedTexts() << methodRE.captureCount();
                if (methodRE.captureCount() == 2) {
                    m_method = methodRE.cap(1);
                    m_args = methodProtocolRE.cap(2).split(QLatin1String("/"));
                } else {
                    badRequest = true;
                }
            }

            if (badRequest) {
                qDebug() << "BAD REQUEST" << request;
                return;
            }
        }
    }

    while ((newLine = request.indexOf('\n', m_bufLastIndex)) != -1) {
        QString section = request.mid(m_bufLastIndex, newLine - m_bufLastIndex - 1);
        m_bufLastIndex = newLine + 1;

        if (!section.isEmpty()) {
            m_headers[section.section(QLatin1Char(':'), 0, 0)] = section.section(QLatin1Char(':'), 1).trimmed();
            qDebug() << section << section.length();
            qDebug() << section.section(QLatin1Char(':'), 0, 0) << section.section(QLatin1Char(':'), 1).trimmed() << endl;
        }
    }

    CutelystRequestPrivate *requestPriv = new CutelystRequestPrivate;
    requestPriv->engine = this;
    requestPriv->method = m_method;
    requestPriv->args = m_args;
    requestPriv->protocol = m_protocol;
    requestPriv->headers = m_headers;

    qDebug() << request;

    //    while (request.end())
    QByteArray data;
    data = "<!DOCTYPE html>"
            "<html lang=\"en\">"
            "    <head>"
            "        <meta charset=\"utf-8\">"
            "        <title>Hello World</title>"
            "    </head>"
            "    <body>"
            "        <h1>Hello World</h1>"
            "        <p>"
            "            Jamie was here."
            "        </p>"
            "    </body>"
            "</html>";
    write(data);
    deleteLater();
}
